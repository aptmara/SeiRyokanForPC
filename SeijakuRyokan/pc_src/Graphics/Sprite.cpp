/*****************************************************************//**
 * @file   Sprite.cpp
 * @brief  PC‘¤‚Ì•`‰æˆ—ŽÀ‘•
 *
 * @author ó–ì—E¶
 * @date   2025/7/16
 *********************************************************************/
#include "sprite.h"
#include "SpriteDrawer.h"

static SpriteDrawer* g_spriteDrawer = nullptr;

void InitSprite(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (!g_spriteDrawer)
    {
        g_spriteDrawer = new SpriteDrawer();
        if (!g_spriteDrawer->Initialize(device, context, 1920, 1080, false))
        {
            OutputDebugString(L"‰Šú‰»‚ÉŽ¸”sI");
        }
    }
}

void UninitSprite()
{
    if (g_spriteDrawer)
    {
        g_spriteDrawer->Finalize();
        delete g_spriteDrawer;
        g_spriteDrawer = nullptr;
    }
}

void DrawSpriteQuad(QUAD_2D* quad)
{
    if (!g_spriteDrawer || !quad || !quad->use || !quad->pTexture) return;

    g_spriteDrawer->Draw(
        quad->pTexture,
        MyGame::Float2(quad->quadPos.x, quad->quadPos.y),
        quad->quadSize,
        quad->quadColor,
        quad->angleDeg,
        quad->posTexCoord,
        quad->sizeTexCoord
    );
}


void DrawSpriteTriangle(TRIANGLE_2D* tri)
{
}
