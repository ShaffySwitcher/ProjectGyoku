#include "Sprite.h"

void Background::render()
{
	if (this->texture) {
		DrawGraph(0, 0, texture->getHandle(), false);
	}
}

void Sprite::render(Vector position)
{
	if (!visible) return;
	if (!texture) return;

	// Calculate sprite dimensions
	const float baseW = (sizeOverride.x > 0.0f) ? sizeOverride.x : texCoords.w;
	const float baseH = (sizeOverride.y > 0.0f) ? sizeOverride.y : texCoords.h;
	const float scaledW = baseW * scale.x;
	const float scaledH = baseH * scale.y;

	// Use texCoords dimensions if set, otherwise use texture dimensions
	const float texW = (texCoords.w > 0.0f) ? texCoords.w : static_cast<float>(texture->getWidth());
	const float texH = (texCoords.h > 0.0f) ? texCoords.h : static_cast<float>(texture->getHeight());

	// Calculate final position
	Vector finalPosition = position + offset;
	if (cornerRelativePlacement) {
		finalPosition.x += scaledW / 2.0f;
		finalPosition.y += scaledH / 2.0f;
	}

	// Build transformation matrices
	const float tiltScaleX = cosf(rotation.y);
	const float tiltScaleY = cosf(rotation.x);

	MATRIX scaleMatrix{};
	CreateScalingMatrix(&scaleMatrix, scaledW * tiltScaleX, scaledH * tiltScaleY, 1.0f);

	MATRIX flipMatrix{};
	CreateScalingMatrix(&flipMatrix, flipX ? -1.0f : 1.0f, flipY ? -1.0f : 1.0f, 1.0f);

	MATRIX rotationMatrix{};
	CreateRotationZMatrix(&rotationMatrix, rotation.z);

	MATRIX translationMatrix{};
	CreateTranslationMatrix(&translationMatrix, finalPosition.x, finalPosition.y, finalPosition.z);

	// Combine: scale -> flip -> rotate -> translate
	MATRIX mat = MMult(scaleMatrix, flipMatrix);
	mat = MMult(mat, rotationMatrix);
	mat = MMult(mat, translationMatrix);

	// Calculate texture coordinates
	const float texLeft = texCoords.x / static_cast<float>(texture->getWidth()) + texOffset.x;
	const float texRight = (texCoords.x + texW) / static_cast<float>(texture->getWidth()) + texOffset.x;
	const float texTop = texCoords.y / static_cast<float>(texture->getHeight()) + texOffset.y;
	const float texBottom = (texCoords.y + texH) / static_cast<float>(texture->getHeight()) + texOffset.y;

	VERTEX2D vertices[6]{};

	vertices[0] = Vertex::createVertex2D(GetVector(-0.5f, -0.5f, 0.0f), 1.0f, color, texLeft, texTop);
	vertices[1] = Vertex::createVertex2D(GetVector(0.5f, -0.5f, 0.0f), 1.0f, color, texRight, texTop);
	vertices[2] = Vertex::createVertex2D(GetVector(-0.5f, 0.5f, 0.0f), 1.0f, color, texLeft, texBottom);
	vertices[3] = Vertex::createVertex2D(GetVector(0.5f, 0.5f, 0.0f), 1.0f, color, texRight, texBottom);
	vertices[4] = vertices[1];
	vertices[5] = vertices[2];

	for (int v = 0; v < 6; v++) {
		vertices[v].pos = VTransform(vertices[v].pos, mat);
	}

	SetUseZBufferFlag(zBufferEnabled);
	SetWriteZBufferFlag(zWriteEnabled);

	SetDrawBlendMode(blendMode, color.alpha);
	SetDrawBright(color.red, color.green, color.blue);

	DrawPolygon2D(vertices, 2, texture->getHandle(), true);

	SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

	SetWriteZBufferFlag(FALSE);
	SetUseZBufferFlag(FALSE);
}
