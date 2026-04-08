#include "Text.h"
#include "Log.h"

int NativeText::defaultFontHandle = -1;
std::shared_ptr<Animation> Text::defaultFontAnimation = nullptr;

TextBase::TextBase(Vector position, std::string text, TextAlign align)
{
	this->position = position;
	this->text = text;
	this->align = align;
}

void TextBase::update()
{
	Drawable::update();
}

NativeText::NativeText(Vector position, std::string text, TextAlign align, bool shadow, TextGradientColor gradient)
    : TextBase(position, text, align)
{
	this->shadow = shadow;
	this->gradient = gradient;
	this->fontHandle = defaultFontHandle;
}

bool NativeText::init()
{
	defaultFontHandle = CreateFontToHandle(NULL, -1, -1, DX_FONTTYPE_ANTIALIASING_8X8);
	if (defaultFontHandle == -1) { return false; }

	return true;
}

void NativeText::restore()
{
	if (!NativeText::init()) {
		Log::error("NativeText::restore(): Failed to restore default font handle.");
	}
}

void NativeText::render(Vector position)
{
	if (!visible) return;

	position = this->position + position + offset;

	if (!CheckFontHandleValid(this->fontHandle)) {
		Log::print("NativeText::render(): Invalid font handle, falling back to default.");
		this->fontHandle = defaultFontHandle;
	}

	TextInfo textInfo = getTextInfo();
	int softImage = MakeARGB8ColorSoftImage(static_cast<int>(textInfo.size.x), static_cast<int>(textInfo.size.y));

	if (softImage != -1) {
		FillSoftImage(softImage, 0, 0, 0, 0);
		BltStringSoftImageToHandle(0, 0, text.c_str(), softImage, -1, defaultFontHandle);

		int graph = CreateGraphFromRectSoftImage(softImage, 0, 0, static_cast<int>(textInfo.size.x), static_cast<int>(textInfo.size.y));
		if (graph != -1) {
			const float lineHeight = (textInfo.lineCount > 0) ? (textInfo.size.y / static_cast<float>(textInfo.lineCount)) : textInfo.size.y;

			const float scaledWidth = textInfo.size.x * getScale().x;
			const float scaledHeight = textInfo.lineCount * lineHeight * getScale().y;

			float alignOffsetX = 0.0f;
			switch (align) {
				case TextAlign::LEFT:   alignOffsetX = 0.0f; break;
				case TextAlign::CENTER: alignOffsetX = -scaledWidth / 2.0f; break;
				case TextAlign::RIGHT:  alignOffsetX = -scaledWidth; break;
			}

			const float tiltScaleX = cosf(rotation.y);
			const float tiltScaleY = cosf(rotation.x);

			MATRIX flipMatrix{};
			CreateScalingMatrix(&flipMatrix, flipX ? -1.0f : 1.0f, flipY ? -1.0f : 1.0f, 1.0f);

			MATRIX rotationMatrix{};
			CreateRotationZMatrix(&rotationMatrix, rotation.z);

			MATRIX translationMatrix{};
			CreateTranslationMatrix(&translationMatrix, position.x, position.y, position.z);

			MATRIX textBlockMatrix = MMult(flipMatrix, rotationMatrix);
			textBlockMatrix = MMult(textBlockMatrix, translationMatrix);

			for (int line = 0; line < textInfo.lineCount; line++) {
			const float startV = (line * lineHeight) / textInfo.size.y;
			const float endV = ((line + 1) * lineHeight) / textInfo.size.y;

			const float lineLocalX = alignOffsetX + scaledWidth / 2.0f;
			const float lineLocalY = (line * lineHeight + lineHeight / 2.0f) * getScale().y;

				const float lineScaleX = textInfo.size.x * getScale().x * tiltScaleX;
				const float lineScaleY = lineHeight * getScale().y * tiltScaleY;

				MATRIX lineScaleMatrix{};
				CreateScalingMatrix(&lineScaleMatrix, lineScaleX, lineScaleY, 0.0f);

				MATRIX lineOffsetMatrix{};
				CreateTranslationMatrix(&lineOffsetMatrix, lineLocalX, lineLocalY, 0.0f);

				MATRIX mat = MMult(lineScaleMatrix, lineOffsetMatrix);
				mat = MMult(mat, textBlockMatrix);

				VERTEX2D vertices[6]{};
				VERTEX2D verticesShadow[6]{};

				vertices[0] = Vertex::createVertex2D(GetVector(-0.5f, -0.5f, 0.0f),
					1.0f, Color::getColor(gradient.top.red, gradient.top.green, gradient.top.blue, gradient.top.alpha),
					0.0f, startV);
				vertices[1] = Vertex::createVertex2D(GetVector(0.5f, -0.5f, 0.0f),
					1.0f, Color::getColor(gradient.top.red, gradient.top.green, gradient.top.blue, gradient.top.alpha),
					1.0f, startV);
				vertices[2] = Vertex::createVertex2D(GetVector(-0.5f, 0.5f, 0.0f),
					1.0f, Color::getColor(gradient.bottom.red, gradient.bottom.green, gradient.bottom.blue, gradient.bottom.alpha),
					0.0f, endV);
				vertices[3] = Vertex::createVertex2D(GetVector(0.5f, 0.5f, 0.0f),
					1.0f, Color::getColor(gradient.bottom.red, gradient.bottom.green, gradient.bottom.blue, gradient.bottom.alpha),
					1.0f, endV);
				vertices[4] = vertices[1];
				vertices[5] = vertices[2];

				verticesShadow[0] = Vertex::createVertex2D(GetVector(-0.5f, -0.5f, 0.0f),
					1.0f, Color::getColor(0, 0, 0, gradient.top.alpha),
					0.0f, startV);
				verticesShadow[1] = Vertex::createVertex2D(GetVector(0.5f, -0.5f, 0.0f),
					1.0f, Color::getColor(0, 0, 0, gradient.top.alpha),
					1.0f, startV);
				verticesShadow[2] = Vertex::createVertex2D(GetVector(-0.5f, 0.5f, 0.0f),
					1.0f, Color::getColor(0, 0, 0, gradient.top.alpha),
					0.0f, endV);
				verticesShadow[3] = Vertex::createVertex2D(GetVector(0.5f, 0.5f, 0.0f),
					1.0f, Color::getColor(0, 0, 0, gradient.top.alpha),
					1.0f, endV);
				verticesShadow[4] = verticesShadow[1];
				verticesShadow[5] = verticesShadow[2];

				for (int v = 0; v < 6; v++) {
					vertices[v].pos = VTransform(vertices[v].pos, mat);

					auto shadowPos = VTransform(verticesShadow[v].pos, mat);
					verticesShadow[v].pos = VAdd(shadowPos, VGet(1.0f, 1.0f, 1.0f)); // shadow offset
				}

				SetUseZBufferFlag(zBufferEnabled);
				SetWriteZBufferFlag(zWriteEnabled);

				SetDrawBlendMode(blendMode, color.alpha);
				SetDrawBright(color.red, color.green, color.blue);

				if (this->shadow) DrawPolygon2D(verticesShadow, 2, graph, TRUE);
				DrawPolygon2D(vertices, 2, graph, TRUE);

				SetDrawBright(255, 255, 255);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

				SetWriteZBufferFlag(FALSE);
				SetUseZBufferFlag(FALSE);
			}

			DeleteGraph(graph);
		}

		DeleteSoftImage(softImage);
	}
}

Text::Text(Vector position, std::string text, TextAlign align, int xSpacing, unsigned int shift)
	: TextBase(position, text, align)
{
	this->xSpacing = xSpacing;
	this->shift = shift;
	this->fontAnimation = defaultFontAnimation;
}

bool Text::init()
{
	defaultFontAnimation = ANMManager::load("text");
	return defaultFontAnimation != nullptr;
}

void Text::render(Vector position)
{
	if (!visible) return;

	position = this->position + position + offset;

	if (!fontAnimation) {
		Log::print("Text::render(): Invalid font handle, falling back to default.");
		this->fontAnimation = defaultFontAnimation;
	}

	std::vector<VERTEX2D> vertices;
	vertices.resize(6 * text.size());

	const float totalWidth = text.size() * xSpacing * getScale().x;

	float alignOffsetX = 0.0f;
	switch (align) {
		case TextAlign::LEFT:   alignOffsetX = 0.0f; break;
		case TextAlign::CENTER: alignOffsetX = -totalWidth / 2.0f; break;
		case TextAlign::RIGHT:  alignOffsetX = -totalWidth; break;
	}

	const float tiltScaleX = cosf(rotation.y);
	const float tiltScaleY = cosf(rotation.x);

	MATRIX flipMatrix{};
	CreateScalingMatrix(&flipMatrix, flipX ? -1.0f : 1.0f, flipY ? -1.0f : 1.0f, 1.0f);

	MATRIX rotationMatrix{};
	CreateRotationZMatrix(&rotationMatrix, rotation.z);

	MATRIX translationMatrix{};
	CreateTranslationMatrix(&translationMatrix, position.x, position.y, position.z);

	MATRIX textBlockMatrix = MMult(flipMatrix, rotationMatrix);
	textBlockMatrix = MMult(textBlockMatrix, translationMatrix);

	for (unsigned int i = 0; i < text.size(); i++) {
		Rect sprite = this->fontAnimation->sprites[text[i] - shift];

		if (text[i] == TEXT_SP_SYMBOL) { sprite = this->fontAnimation->sprites[107]; }
		if (text[i] == TEXT_END_SYMBOL) { sprite = this->fontAnimation->sprites[108]; }

		const float charLocalX = alignOffsetX + (i * xSpacing + sprite.w / 2.0f) * getScale().x;
		const float charLocalY = (sprite.h / 2.0f) * getScale().y;

		const float charScaleX = sprite.w * getScale().x * tiltScaleX;
		const float charScaleY = sprite.h * getScale().y * tiltScaleY;

		MATRIX charScaleMatrix{};
		CreateScalingMatrix(&charScaleMatrix, charScaleX, charScaleY, 0.0f);

		MATRIX charOffsetMatrix{};
		CreateTranslationMatrix(&charOffsetMatrix, charLocalX, charLocalY, 0.0f);

		MATRIX matrix = MMult(charScaleMatrix, charOffsetMatrix);
		matrix = MMult(matrix, textBlockMatrix);

		Point texturePositions[4]{
			GetPoint(((sprite.x) / (float)this->fontAnimation->texture.getWidth()),((sprite.y) / (float)this->fontAnimation->texture.getHeight())),
			GetPoint(((sprite.x + sprite.w) / (float)this->fontAnimation->texture.getWidth()),((sprite.y) / (float)this->fontAnimation->texture.getHeight())),
			GetPoint(((sprite.x) / (float)this->fontAnimation->texture.getWidth()),((sprite.y + sprite.h) / (float)this->fontAnimation->texture.getHeight())),
			GetPoint(((sprite.x + sprite.w) / (float)this->fontAnimation->texture.getWidth()),((sprite.y + sprite.h) / (float)this->fontAnimation->texture.getHeight()))
		};

		vertices[0 + i * 6] = Vertex::createVertex2D(GetVector(-0.5f, -0.5f, 0.0f),
			1.0f, this->color,
			texturePositions[0].x, texturePositions[0].y);
		vertices[1 + i * 6] = Vertex::createVertex2D(GetVector(0.5f, -0.5f, 0.0f),
			1.0f, this->color,
			texturePositions[1].x, texturePositions[1].y);
		vertices[2 + i * 6] = Vertex::createVertex2D(GetVector(-0.5f, 0.5f, 0.0f),
			1.0f, this->color,
			texturePositions[2].x, texturePositions[2].y);
		vertices[3 + i * 6] = Vertex::createVertex2D(GetVector(0.5f, 0.5f, 0.0f),
			1.0f, this->color,
			texturePositions[3].x, texturePositions[3].y);
		vertices[4 + i * 6] = vertices[1 + i * 6];
		vertices[5 + i * 6] = vertices[2 + i * 6];

		for (int j = 0; j < 6; j++) {
			vertices[i * 6 + j].pos = VTransform(vertices[i * 6 + j].pos, matrix);
		}
	}

	SetUseZBufferFlag(zBufferEnabled);
	SetWriteZBufferFlag(zWriteEnabled);

	SetDrawBlendMode(blendMode, color.alpha);
	SetDrawBright(this->color.red, this->color.green, this->color.blue);

	DrawPolygon2D(vertices.data(), 2 * text.size(), this->fontAnimation->texture.getHandle(), true);

	SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

	SetWriteZBufferFlag(false);
	SetUseZBufferFlag(false);
}
