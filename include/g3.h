#ifndef BHDS_G3_H
#define BHDS_G3_H

/*
 * NitroSDK geometry engine helpers (G3_*), written as the SDK's inline
 * functions: the game was built with them, and calling them through the
 * inline (rather than writing the packed value directly) is what gives the
 * original register allocation.
 */
#include "types.h"
#include "hw_regs.h"

#define GX_TEXFMT_PLTT4 2

static inline void G3_PolygonAttr(int light, int polyMode, int cullMode, int polygonID, int alpha, int misc)
{
    REG_POLYGON_ATTR = (u32)(((light) << 0) | ((polyMode) << 4) | ((cullMode) << 6) | (misc) |
                             ((polygonID) << 24) | ((alpha) << 16));
}

static inline void G3_TexPlttBase(u32 addr, int texFmt)
{
    REG_PLTT_BASE = (u32)((addr) >> (4 - ((texFmt) == GX_TEXFMT_PLTT4)));
}

static inline void G3_MaterialColorDiffAmb(u32 diffuse, u32 ambient, BOOL isSetVtxColor)
{
    REG_DIF_AMB = (u32)((diffuse) | ((ambient) << 16) | ((isSetVtxColor) << 15));
}

static inline void G3_MaterialColorSpecEmi(u32 specular, u32 emission, BOOL isShininess)
{
    REG_SPE_EMI = (u32)((specular) | ((emission) << 16) | ((isShininess) << 15));
}

#endif /* BHDS_G3_H */
