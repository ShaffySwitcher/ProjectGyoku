#include "Engine/Graphics/Animation.h"
#include "Engine/Log.h"
#include "Engine/FileManager.h"

#include <algorithm>
#include <unordered_map>
#include "Engine/Graphics/Sprite.h"

std::map<std::string, std::shared_ptr<Animation>> ANMManager::animations = {};
std::vector<ANMRunner*> ANMRunner::activeRunners = {};

const std::map<std::string, std::string> animationPaths = {
	{ "dummy", "data/dummy.anm" },
	{ "text", "data/text/text.anm" },
	{ "player", "data/player/player.anm" }
};

std::shared_ptr<Animation> Animation::loadFromFile(const std::string& path, bool loadTexture)
{
	auto anm = std::make_shared<Animation>();
	anm->path = path;

	std::shared_ptr<FileBuffer> anmFile = FileManager::loadFile(path);
	if(!anmFile){
		Log::error("Animation::loadFromFile(): Failed to load animation file: %s", path.c_str());
		return nullptr;
	}

	ANMHeader header;
	anmFile->readBytes(&header, sizeof(ANMHeader));

	if (strncmp(header.magic, "ANIM", 4) != 0) {
		Log::error("Animation::loadFromFile(): Corrupt file: %s", path.c_str());
		return nullptr;
	}

	if (header.version != ENGINE_VERSION) {
		Log::error("Animation::loadFromFile(): Unsupported ANM version %d in file: %s", header.version, path.c_str());
		return nullptr;
	}

	anmFile->seek(header.spriteTableOffset);
	for (uint32_t i = 0; i < header.numSprites; i++) {
		int32_t id = anmFile->read<int32_t>();
		Rect rectangle = anmFile->read<Rect>();

		anm->sprites[id] = rectangle;
	}

	anmFile->seek(header.scriptTableOffset);
	for (uint32_t i = 0; i < header.numScripts; i++) {
		int32_t id = anmFile->read<int32_t>();
		uint32_t scriptOffset = anmFile->read<uint32_t>();

		ANMScript& script = anm->scripts[id];

		size_t offset = anmFile->tell();

		anmFile->seek(scriptOffset);

		while (true) {
			ANMInstruction instruction;

			instruction.offset = anmFile->tell() - scriptOffset;
			instruction.time = anmFile->read<uint16_t>();
			instruction.opcode = static_cast<ANMOpcode>(anmFile->read<uint8_t>());

			uint8_t size = anmFile->read<uint8_t>();
			instruction.args.resize(size);
			anmFile->readBytes(instruction.args.data(), size);

			script.instructions.push_back(instruction);

			if (instruction.opcode == ANMOpcode::STOP) {
				break;
			}
		}

		std::unordered_map<uint32_t, uint32_t> offsetToIndex;

		for (uint32_t i = 0; i < script.instructions.size(); i++) {
			offsetToIndex[script.instructions[i].offset] = i;
		}

		for (uint32_t i = 0; i < script.instructions.size(); i++) {
			auto& instr = script.instructions[i];

			switch (instr.opcode) {
				case ANMOpcode::INTERRUPT_LABEL: {
					int32_t label = instr.get<int32_t>();
					script.interrupts[label] = i;
					break;
				}
			}
		}

		anmFile->seek(offset);
	}

	if (loadTexture && header.pathOffset) {
		std::string textureAlphaPath{};
		std::string texturePath{};

		if (header.pathOffsetAlpha) {
			anmFile->seek(header.pathOffsetAlpha);
			textureAlphaPath = anmFile->readStr();
		}

		
		anmFile->seek(header.pathOffset);
		texturePath = anmFile->readStr();

		anm->texture.loadTexture(texturePath.c_str(), header.pathOffsetAlpha ? textureAlphaPath.c_str() : "");
	}

	return anm;
}

std::shared_ptr<Animation> ANMManager::load(const std::string& name, const std::string& path)
{
	auto it = animations.find(name);
	if(it != animations.end()) {
		return it->second;
	}

	const std::string* resolvedPath = &path;
	if (resolvedPath->empty()) {
		auto pathIt = animationPaths.find(name);
		if (pathIt == animationPaths.end()) {
			Log::print("ANMManager::load(): No path provided for animation '%s' and no fallback entry was found", name.c_str());
			return nullptr;
		}

		resolvedPath = &pathIt->second;
	}

	auto anm = Animation::loadFromFile(*resolvedPath);
	if (!anm) {
		Log::print("ANMManager::load(): Failed to load ANM: %s", resolvedPath->c_str());
		return nullptr;
	}

	anm->path = *resolvedPath;

	animations[name] = anm;
	return anm;
}

void ANMManager::unload(const std::string& name)
{
	auto it = animations.find(name);
	if(it != animations.end()) {
		animations.erase(it);
	}
}

void ANMManager::unloadAll()
{
	animations.clear();
}

bool ANMManager::reloadScripts(const std::string& name)
{
	auto it = animations.find(name);
	if (it == animations.end() || !it->second) {
		return false;
	}

	auto& animation = it->second;
	if (animation->path.empty()) {
		Log::print("ANMManager::reloadScripts(): Animation '%s' has no source path", name.c_str());
		return false;
	}

	auto refreshed = Animation::loadFromFile(animation->path, false);
	if (!refreshed) {
		Log::print("ANMManager::reloadScripts(): Failed to reload scripts for '%s'", name.c_str());
		return false;
	}

	animation->sprites = std::move(refreshed->sprites);
	animation->scripts = std::move(refreshed->scripts);
	animation->revision++;

	Log::print("ANMManager::reloadScripts(): Reloaded scripts for '%s'", name.c_str());
	return true;
}

int ANMManager::reloadScripts()
{
	int reloaded = 0;
	for (const auto& entry : animations) {
		if (reloadScripts(entry.first)) {
			reloaded++;
		}
	}

	return reloaded;
}

int ANMManager::reloadScriptsAndRestartRunners()
{
	const int reloaded = reloadScripts();
	ANMRunner::restartAll();
	return reloaded;
}

int ANMManager::reloadTextures()
{
	int reloaded = 0;
	for (auto& pair : animations) {
		if (pair.second && pair.second->texture.restore()) {
			reloaded++;
		}
	}

	return reloaded;
}

void ANMManager::restore()
{
	reloadTextures();
}

ANMRunner::ANMRunner(std::shared_ptr<Animation> animation, uint32_t id, std::shared_ptr<Drawable> target, uint32_t spriteOffset)
{
	this->animation = animation;
	this->target = target;
	this->spriteOffset = spriteOffset;
	this->scriptId = static_cast<int32_t>(id);
	activeRunners.push_back(this);

	bindScript(true);

	this->step();
	this->spriteOffset = 0;
}

ANMRunner::~ANMRunner()
{
	auto it = std::find(activeRunners.begin(), activeRunners.end(), this);
	if (it != activeRunners.end()) {
		activeRunners.erase(it);
	}
}

void ANMRunner::restart()
{
	running = true;
	waiting = false;
	instructionIndex = 0;
	frame.reset();

	if (!bindScript(true)) {
		return;
	}

	step();
}

void ANMRunner::restartAll()
{
	for (ANMRunner* runner : activeRunners) {
		if (runner) {
			runner->restart();
		}
	}
}

bool ANMRunner::bindScript(bool logOnFailure)
{
	if (!animation) {
		script = nullptr;
		return false;
	}

	auto it = animation->scripts.find(scriptId);
	if (it == animation->scripts.end()) {
		script = nullptr;
		if (logOnFailure && !missingScriptLogged) {
			Log::print("ANMRunner::bindScript(): Animation does not contain script with ID %d", scriptId);
			missingScriptLogged = true;
		}
		return false;
	}

	script = &it->second;
	boundRevision = animation->revision;
	missingScriptLogged = false;

	if (script->instructions.empty()) {
		running = false;
		instructionIndex = 0;
		return false;
	}

	if (instructionIndex >= script->instructions.size()) {
		instructionIndex = static_cast<uint32_t>(script->instructions.size() - 1);
	}

	return true;
}

void ANMRunner::refreshScriptBinding()
{
	if (animation && boundRevision != animation->revision) {
		bindScript(true);
	}
}

bool ANMRunner::step()
{
	refreshScriptBinding();

	if(!script || !target) {
		if (target) {
			target->update();
		}
		return false;
	}

	if (script->instructions.empty()) {
		running = false;
		target->update();
		return false;
	}

	if (instructionIndex >= script->instructions.size()) {
		instructionIndex = static_cast<uint32_t>(script->instructions.size() - 1);
		running = false;
		target->update();
		return false;
	}

	while (running && !waiting) {
		if (instructionIndex >= script->instructions.size()) {
			running = false;
			break;
		}

		ANMInstruction* instr = &script->instructions[instructionIndex];

		if (instr->time > this->frame.getFrame()) {
			break;
		}
		else {
			instructionIndex++;
		}

		if (instr->time == this->frame.getFrame()) {
			switch (instr->opcode) {
				case ANMOpcode::NOP:
					break;
				case ANMOpcode::SET_SPRITE: {
					setSprite(instr->as<SetSpriteArgs>().id);
					break;
				}
				case ANMOpcode::SET_RANDOM_SPRITE: {
					const auto args = instr->as<SetRandomSpriteArgs>();
					setSprite(gRng.getIntRange(args.min_id, args.min_id + args.amp));
					break;
				}
				case ANMOpcode::SET_OFFSET: {
					const auto args = instr->as<SetOffsetArgs>();
					setOffset(args.ox, args.oy, args.oz);
					break;
				}
				case ANMOpcode::SET_SCALE: {
					const auto args = instr->as<SetScaleArgs>();
					setScale(args.sx, args.sy);
					break;
				}
				case ANMOpcode::SET_ROTATION: {
					const auto args = instr->as<SetRotationArgs>();
					setRotation(args.rx, args.ry, args.rz);
					break;
				}
				case ANMOpcode::SET_COLOR: {
					const auto args = instr->as<SetColorArgs>();
					setColor(args.r, args.g, args.b);
					break;
				}
				case ANMOpcode::SET_ALPHA: {
					const auto args = instr->as<SetAlphaArgs>();
					setAlpha(args.alpha);
					break;
				}
				case ANMOpcode::SET_VISIBILITY: {
					const auto args = instr->as<SetVisibilityArgs>();
					setVisibility(args.visible != 0);
					break;
				}
				case ANMOpcode::SET_BLEND_MODE: {
					const auto args = instr->as<SetBlendModeArgs>();
					setBlendMode(args.mode);
					break;
				}
				case ANMOpcode::SCROLL_TEXTURE_X: {
					const auto args = instr->as<ScrollTextureXArgs>();
					scrollTexture(args.dx, 0.0f);
					break;
				}
				case ANMOpcode::SCROLL_TEXTURE_Y: {
					const auto args = instr->as<ScrollTextureYArgs>();
					scrollTexture(0.0f, args.dy);
					break;
				}
				case ANMOpcode::FLIP_X:
					flipX(true);
					break;
				case ANMOpcode::FLIP_Y:
					flipY(true);
					break;
				case ANMOpcode::ANCHOR_TOP_LEFT:
					setAnchorTopLeft(true);
					break;
				case ANMOpcode::SET_OFFSET_SPEED: {
					const auto args = instr->as<SetOffsetSpeedArgs>();
					setOffsetSpeed(args.osx, args.osy, args.osz);
					break;
				}
				case ANMOpcode::SET_SCALE_SPEED: {
					const auto args = instr->as<SetScaleSpeedArgs>();
					setScaleSpeed(args.ssx, args.ssy);
					break;
				}
				case ANMOpcode::SET_ROTATION_SPEED: {
					const auto args = instr->as<SetRotationSpeedArgs>();
					setRotationSpeed(args.srx, args.sry, args.srz);
					break;
				}
				case ANMOpcode::Z_WRITE_DISABLE: {
					const auto args = instr->as<ZWriteDisableArgs>();
					setZWriteEnabled(args.enabled != 0);
					break;
				}
				case ANMOpcode::STOP:
					running = false;
					break;
				case ANMOpcode::PAUSE:
					waiting = true;
					break;
				case ANMOpcode::JUMP: {
					const auto args = instr->as<JumpArgs>();
					instructionIndex = std::min<uint32_t>(args.offset, static_cast<uint32_t>(this->script->instructions.size() - 1));
					this->frame = this->script->instructions[instructionIndex].time;
					break;
				}
				case ANMOpcode::FADE_TO: {
					const auto args = instr->as<FadeArgs>();
					fadeTo(args.alpha, args.duration, args.mode);
					break;
				}
				case ANMOpcode::MOVE_TO: {
					const auto args = instr->as<MoveToArgs>();
					moveTo(args.x, args.y, args.z, args.duration, args.mode);
					break;
				}
				case ANMOpcode::ROTATE_TO: {
					const auto args = instr->as<RotateToArgs>();
					rotateTo(args.rx, args.ry, args.rz, args.duration, args.mode);
					break;
				}
				case ANMOpcode::SCALE_TO: {
					const auto args = instr->as<ScaleToArgs>();
					scaleTo(args.sx, args.sy, args.duration, args.mode);
					break;
				}
				case ANMOpcode::INTERRUPT_LABEL:
					break;
				default:
					Log::print("ANMRunner::step(): Unimplemented opcode %d at instruction index %u", static_cast<uint8_t>(instr->opcode), instructionIndex - 1);
					running = false;
					break;
			}
		}
	}

	if (!waiting) {
		this->frame.step();
	}

	target->update();

	return running;
}

void ANMRunner::interrupt(int32_t label, bool setVisible)
{
	refreshScriptBinding();

	if (!script || script->instructions.empty()) {
		return;
	}

	if (script->interrupts.count(label)) {
		this->instructionIndex = script->interrupts[label];
		if (this->instructionIndex >= this->script->instructions.size()) {
			this->instructionIndex = static_cast<uint32_t>(this->script->instructions.size() - 1);
		}
		this->frame = this->script->instructions[instructionIndex].time;

		waiting = false;
		if (setVisible) {
			this->target->setVisible(true);
		}
	}
	else if (script->interrupts.count(-1)) {
		this->interrupt(-1);
	}
}

void ANMRunner::setSprite(int32_t id)
{
	if (auto sprite = std::dynamic_pointer_cast<Sprite>(this->target)) {
		try {
			Rect spriteRect = this->animation->sprites.at(id + this->spriteOffset);
			sprite->setTexCoords(spriteRect);
			sprite->setTexture(&this->animation->texture);
		}
		catch (const std::out_of_range&) {
			Log::print("ANMRunner::setSprite(): Animation does not contain sprite with ID %d", id + this->spriteOffset);
		}
	}
	else {
		Log::print("ANMRunner::setSprite(): Target is not a Sprite(?)");
	}
}

void ANMRunner::scrollTexture(float dx, float dy) {
	if (auto sprite = std::dynamic_pointer_cast<Sprite>(this->target)) {
		sprite->setTexOffset(sprite->getTexOffset() + GetPoint(dx, dy));
	}
	else {
		Log::print("ANMRunner::scrollTexture(): Target is not a Sprite(?)");
	}
}