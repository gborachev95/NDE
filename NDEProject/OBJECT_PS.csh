#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer DIRECTION_LIGHT
// {
//
//   struct LIGHT_DATA
//   {
//       
//       float4 transform;              // Offset:    0
//       float3 direction;              // Offset:   16
//       float radius;                  // Offset:   28
//       float4 color;                  // Offset:   32
//
//   } dir_light;                       // Offset:    0 Size:    48
//
// }
//
// cbuffer POINT_LIGHT
// {
//
//   struct LIGHT_DATA
//   {
//       
//       float4 transform;              // Offset:    0
//       float3 direction;              // Offset:   16
//       float radius;                  // Offset:   28
//       float4 color;                  // Offset:   32
//
//   } point_light[125];                // Offset:    0 Size:  6000
//
// }
//
// cbuffer SCENE
// {
//
//   float4x4 viewMatrix;               // Offset:    0 Size:    64
//   float4x4 projectionMatrix;         // Offset:   64 Size:    64 [unused]
//   float4 cameraPosition;             // Offset:  128 Size:    16
//
// }
//
// Resource bind info for structBufferTexture
// {
//
//   struct STRUCT_BUFFER
//   {
//       
//       int lightIndex;                // Offset:    0
//       float4 lightColor;             // Offset:    4
//
//   } $Element;                        // Offset:    0 Size:    20
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// filter                            sampler      NA          NA    0        1
// baseTexture                       texture  float4          2d    0        1
// normalTexture                     texture  float4          2d    1        1
// specularTexture                   texture  float4          2d    2        1
// structBufferTexture               texture  struct         r/o    4        1
// DIRECTION_LIGHT                   cbuffer      NA          NA    2        1
// POINT_LIGHT                       cbuffer      NA          NA    3        1
// SCENE                             cbuffer      NA          NA    5        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float       
// POSITION                 0   xyzw        1     NONE   float   xyz 
// NORMALS                  0   xyzw        2     NONE   float   xyzw
// UV                       0   xyzw        3     NONE   float   xyz 
// TANGENTS                 0   xyzw        4     NONE   float   xyzw
// BITANGENTS               0   xyzw        5     NONE   float   xyzw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_TARGET                0   xyzw        0   TARGET   float   xyzw
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb2[3], immediateIndexed
dcl_constantbuffer cb3[375], dynamicIndexed
dcl_constantbuffer cb5[9], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_resource_texture2d (float,float,float,float) t2
dcl_resource_structured t4, 20 
dcl_input_ps linear v1.xyz
dcl_input_ps linear v2.xyzw
dcl_input_ps linear v3.xyz
dcl_input_ps linear v4.xyzw
dcl_input_ps linear v5.xyzw
dcl_output o0.xyzw
dcl_temps 9
sample_indexable(texture2d)(float,float,float,float) r0.xyzw, v3.xyxx, t0.xyzw, s0
dp4 r1.x, v2.xyzw, v2.xyzw
rsq r1.x, r1.x
mul r1.xyz, r1.xxxx, v2.xyzx
dp4 r1.w, v4.xyzw, v4.xyzw
rsq r1.w, r1.w
mul r2.xyz, r1.wwww, v4.xyzx
dp4 r1.w, v5.xyzw, v5.xyzw
rsq r1.w, r1.w
mul r3.xyz, r1.wwww, v5.xyzx
sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.xyxx, t1.xyzw, s0
mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
mul r3.xyz, r3.xyzx, r4.yyyy
mad r2.xyz, r4.xxxx, r2.xyzx, r3.xyzx
mad r1.xyz, r4.zzzz, r1.xyzx, r2.xyzx
mov r1.w, l(1.000000)
dp4 r1.w, r1.xyzw, r1.xyzw
rsq r1.w, r1.w
mul r1.xyz, r1.wwww, r1.xyzx
sample_indexable(texture2d)(float,float,float,float) r2.xyzw, v3.xyxx, t2.xyzw, s0
lt r1.w, l(0.000000), cb2[2].w
dp3_sat r3.x, -cb2[1].xyzx, r1.xyzx
mul r3.xyzw, r0.xyzw, r3.xxxx
add r4.xyz, -v1.xyzx, cb5[8].xyzx
dp3 r4.w, r4.xyzx, r4.xyzx
rsq r4.w, r4.w
mul r4.xyz, r4.wwww, r4.xyzx
dp3 r4.w, cb2[1].xyzx, r1.xyzx
add r4.w, r4.w, r4.w
mad r5.xyz, r1.xyzx, -r4.wwww, cb2[1].xyzx
dp3 r4.w, r5.xyzx, r5.xyzx
rsq r4.w, r4.w
mul r5.xyz, r4.wwww, r5.xyzx
dp3 r4.x, r4.xyzx, r5.xyzx
lt r4.y, l(0.000000), r4.x
log r4.x, |r4.x|
mul r4.x, r4.x, l(1000.000000)
exp r4.x, r4.x
mul r6.xyzw, r2.xyzw, r4.xxxx
mad r6.xyzw, r6.xyzw, v3.zzzz, l(0.000000, 0.000000, 0.000000, 1.000000)
movc r4.xyzw, r4.yyyy, r6.xyzw, l(0,0,0,1.000000)
mov_sat r6.xyzw, r0.xyzw
mad r3.xyzw, r3.xyzw, cb2[2].xyzw, r6.xyzw
mul r3.xyz, r3.xyzx, cb2[2].wwww
movc r3.xyzw, r1.wwww, r3.xyzw, l(0,0,0,1.000000)
movc r4.xyzw, r1.wwww, r4.xyzw, l(0,0,0,1.000000)
add r6.xyz, v1.zyxz, l(178.151398, 14.982761, 111.172050, 0.000000)
mul r6.xyz, r6.xyzx, l(0.017888, 0.034676, 0.018580, 0.000000)
ftoi r6.xyz, r6.xyzx
imul null, r6.xy, r6.xyxx, l(25, 5, 0, 0)
iadd r1.w, r6.y, r6.x
iadd r1.w, r6.z, r1.w
ld_structured_indexable(structured_buffer, stride=20)(mixed,mixed,mixed,mixed) r1.w, r1.w, l(0), t4.xxxx
imul null, r1.w, r1.w, l(3)
lt r5.w, l(0.000000), cb3[r1.w + 2].w
if_nz r5.w
  add r6.xyz, -v1.xyzx, cb3[r1.w + 0].xyzx
  dp3 r5.w, r6.xyzx, r6.xyzx
  add r6.w, cb3[r1.w + 1].w, cb3[r1.w + 1].w
  div r6.w, r5.w, r6.w
  add_sat r6.w, -r6.w, l(1.000000)
  rsq r7.x, r5.w
  mul r7.yzw, r6.xxyz, r7.xxxx
  mad r8.x, r6.x, r7.x, cb5[0].z
  mad r8.y, r6.y, r7.x, cb5[1].z
  mad r8.z, r6.z, r7.x, cb5[2].z
  dp3 r6.x, r8.xyzx, r8.xyzx
  rsq r6.x, r6.x
  mul r6.xyz, r6.xxxx, r8.xyzx
  dp3_sat r1.x, r7.yzwy, r1.xyzx
  div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r5.w
  mul r1.y, r1.y, cb3[r1.w + 1].w
  mul r1.x, r1.y, r1.x
  dp3 r1.y, r6.xyzx, r5.xyzx
  lt r1.z, l(0.000000), r1.y
  log r1.y, |r1.y|
  mul r1.y, r1.y, l(128.000000)
  exp r1.y, r1.y
  mul r2.xyzw, r2.xyzw, r1.yyyy
  mul r2.xyzw, r6.wwww, r2.xyzw
  mad r2.xyzw, r2.xyzw, v3.zzzz, r4.xyzw
  movc r2.xyzw, r1.zzzz, r2.xyzw, r4.xyzw
  mul r0.xyzw, r0.xyzw, cb3[r1.w + 2].xyzw
  mul r0.xyzw, r6.wwww, r0.xyzw
  mul_sat r0.xyzw, r1.xxxx, r0.xyzw
  mul r0.xyzw, r0.xyzw, cb3[r1.w + 2].wwww
  eq r1.x, cb2[2].w, l(0.000000)
  eq r1.y, l(0.000000), cb3[r1.w + 2].w
  and r1.x, r1.y, r1.x
  movc r4.xyzw, r1.xxxx, l(0,0,0,0), r2.xyzw
else 
  mov r0.xyzw, l(0,0,0,1.000000)
endif 
add r0.xyzw, r0.xyzw, r3.xyzw
mov_sat r4.xyzw, r4.xyzw
add_sat r0.xyzw, r0.xyzw, r4.xyzw
lt r1.x, r0.w, l(0.100000)
discard_nz r1.x
mov o0.xyzw, r0.xyzw
ret 
// Approximately 100 instruction slots used
#endif

const BYTE OBJECT_PS[] =
{
     68,  88,  66,  67,  27, 187, 
     44, 200,  10, 197,  52, 140, 
     27,  32,  85, 162, 185,  15, 
    182, 183,   1,   0,   0,   0, 
     76,  20,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    220,   5,   0,   0, 176,   6, 
      0,   0, 228,   6,   0,   0, 
    176,  19,   0,   0,  82,  68, 
     69,  70, 160,   5,   0,   0, 
      4,   0,   0,   0, 164,   1, 
      0,   0,   8,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    255, 255,   0,   1,   0,   0, 
    108,   5,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
     60,   1,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  67,   1,   0,   0, 
      2,   0,   0,   0,   5,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     13,   0,   0,   0,  79,   1, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  13,   0,   0,   0, 
     93,   1,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   2,   0,   0,   0, 
      1,   0,   0,   0,  13,   0, 
      0,   0, 109,   1,   0,   0, 
      5,   0,   0,   0,   6,   0, 
      0,   0,   1,   0,   0,   0, 
     20,   0,   0,   0,   4,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 129,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    145,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 157,   1,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 102, 105, 
    108, 116, 101, 114,   0,  98, 
     97, 115, 101,  84, 101, 120, 
    116, 117, 114, 101,   0, 110, 
    111, 114, 109,  97, 108,  84, 
    101, 120, 116, 117, 114, 101, 
      0, 115, 112, 101,  99, 117, 
    108,  97, 114,  84, 101, 120, 
    116, 117, 114, 101,   0, 115, 
    116, 114, 117,  99, 116,  66, 
    117, 102, 102, 101, 114,  84, 
    101, 120, 116, 117, 114, 101, 
      0,  68,  73,  82,  69,  67, 
     84,  73,  79,  78,  95,  76, 
     73,  71,  72,  84,   0,  80, 
     79,  73,  78,  84,  95,  76, 
     73,  71,  72,  84,   0,  83, 
     67,  69,  78,  69,   0, 171, 
    129,   1,   0,   0,   1,   0, 
      0,   0,   4,   2,   0,   0, 
     48,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    145,   1,   0,   0,   1,   0, 
      0,   0,  64,   3,   0,   0, 
    112,  23,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    157,   1,   0,   0,   3,   0, 
      0,   0, 152,   3,   0,   0, 
    144,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    109,   1,   0,   0,   1,   0, 
      0,   0, 140,   4,   0,   0, 
     20,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     44,   2,   0,   0,   0,   0, 
      0,   0,  48,   0,   0,   0, 
      2,   0,   0,   0,  28,   3, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 100, 105, 
    114,  95, 108, 105, 103, 104, 
    116,   0,  76,  73,  71,  72, 
     84,  95,  68,  65,  84,  65, 
      0, 116, 114,  97, 110, 115, 
    102, 111, 114, 109,   0, 102, 
    108, 111,  97, 116,  52,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  75,   2, 
      0,   0, 100, 105, 114, 101, 
     99, 116, 105, 111, 110,   0, 
    102, 108, 111,  97, 116,  51, 
      0, 171, 171, 171,   1,   0, 
      3,   0,   1,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    130,   2,   0,   0, 114,  97, 
    100, 105, 117, 115,   0, 102, 
    108, 111,  97, 116,   0, 171, 
    171, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 183,   2, 
      0,   0,  99, 111, 108, 111, 
    114,   0, 171, 171,  65,   2, 
      0,   0,  84,   2,   0,   0, 
      0,   0,   0,   0, 120,   2, 
      0,   0, 140,   2,   0,   0, 
     16,   0,   0,   0, 176,   2, 
      0,   0, 192,   2,   0,   0, 
     28,   0,   0,   0, 228,   2, 
      0,   0,  84,   2,   0,   0, 
     32,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,  12,   0, 
      0,   0,   4,   0, 236,   2, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   2,   0,   0, 104,   3, 
      0,   0,   0,   0,   0,   0, 
    112,  23,   0,   0,   2,   0, 
      0,   0, 116,   3,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 112, 111, 105, 110, 
    116,  95, 108, 105, 103, 104, 
    116,   0,   5,   0,   0,   0, 
      1,   0,  12,   0, 125,   0, 
      4,   0, 236,   2,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   2, 
      0,   0,  16,   4,   0,   0, 
      0,   0,   0,   0,  64,   0, 
      0,   0,   2,   0,   0,   0, 
     36,   4,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     72,   4,   0,   0,  64,   0, 
      0,   0,  64,   0,   0,   0, 
      0,   0,   0,   0,  36,   4, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  89,   4, 
      0,   0, 128,   0,   0,   0, 
     16,   0,   0,   0,   2,   0, 
      0,   0, 104,   4,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 118, 105, 101, 119, 
     77,  97, 116, 114, 105, 120, 
      0, 102, 108, 111,  97, 116, 
     52, 120,  52,   0,   3,   0, 
      3,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     27,   4,   0,   0, 112, 114, 
    111, 106, 101,  99, 116, 105, 
    111, 110,  77,  97, 116, 114, 
    105, 120,   0,  99,  97, 109, 
    101, 114,  97,  80, 111, 115, 
    105, 116, 105, 111, 110,   0, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  75,   2,   0,   0, 
    180,   4,   0,   0,   0,   0, 
      0,   0,  20,   0,   0,   0, 
      2,   0,   0,   0,  72,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  36,  69, 
    108, 101, 109, 101, 110, 116, 
      0,  83,  84,  82,  85,  67, 
     84,  95,  66,  85,  70,  70, 
     69,  82,   0, 108, 105, 103, 
    104, 116,  73, 110, 100, 101, 
    120,   0, 105, 110, 116,   0, 
    171, 171,   0,   0,   2,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 214,   4, 
      0,   0, 108, 105, 103, 104, 
    116,  67, 111, 108, 111, 114, 
      0, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  75,   2, 
      0,   0, 203,   4,   0,   0, 
    220,   4,   0,   0,   0,   0, 
      0,   0,   0,   5,   0,   0, 
     12,   5,   0,   0,   4,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,   5,   0,   0,   0, 
      2,   0,  48,   5,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 189,   4, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  54,  46,  51,  46,  57, 
     54,  48,  48,  46,  49,  54, 
     51,  56,  52,   0, 171, 171, 
     73,  83,  71,  78, 204,   0, 
      0,   0,   6,   0,   0,   0, 
      8,   0,   0,   0, 152,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0, 164,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     15,   7,   0,   0, 173,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     15,  15,   0,   0, 181,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
     15,   7,   0,   0, 184,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
     15,  15,   0,   0, 193,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
     15,  15,   0,   0,  83,  86, 
     95,  80,  79,  83,  73,  84, 
     73,  79,  78,   0,  80,  79, 
     83,  73,  84,  73,  79,  78, 
      0,  78,  79,  82,  77,  65, 
     76,  83,   0,  85,  86,   0, 
     84,  65,  78,  71,  69,  78, 
     84,  83,   0,  66,  73,  84, 
     65,  78,  71,  69,  78,  84, 
     83,   0,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  65,  82, 
     71,  69,  84,   0, 171, 171, 
     83,  72,  69,  88, 196,  12, 
      0,   0,  80,   0,   0,   0, 
     49,   3,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
     89,   8,   0,   4,  70, 142, 
     32,   0,   3,   0,   0,   0, 
    119,   1,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      5,   0,   0,   0,   9,   0, 
      0,   0,  90,   0,   0,   3, 
      0,  96,  16,   0,   0,   0, 
      0,   0,  88,  24,   0,   4, 
      0, 112,  16,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
     88,  24,   0,   4,   0, 112, 
     16,   0,   1,   0,   0,   0, 
     85,  85,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      2,   0,   0,   0,  85,  85, 
      0,   0, 162,   0,   0,   4, 
      0, 112,  16,   0,   4,   0, 
      0,   0,  20,   0,   0,   0, 
     98,  16,   0,   3, 114,  16, 
     16,   0,   1,   0,   0,   0, 
     98,  16,   0,   3, 242,  16, 
     16,   0,   2,   0,   0,   0, 
     98,  16,   0,   3, 114,  16, 
     16,   0,   3,   0,   0,   0, 
     98,  16,   0,   3, 242,  16, 
     16,   0,   4,   0,   0,   0, 
     98,  16,   0,   3, 242,  16, 
     16,   0,   5,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   9,   0, 
      0,   0,  69,   0,   0, 139, 
    194,   0,   0, 128,  67,  85, 
     21,   0, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  16, 
     16,   0,   3,   0,   0,   0, 
     70, 126,  16,   0,   0,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,  17,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  70,  30, 
     16,   0,   2,   0,   0,   0, 
     70,  30,  16,   0,   2,   0, 
      0,   0,  68,   0,   0,   5, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  56,   0, 
      0,   7, 114,   0,  16,   0, 
      1,   0,   0,   0,   6,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  18,  16,   0,   2,   0, 
      0,   0,  17,   0,   0,   7, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  70,  30,  16,   0, 
      4,   0,   0,   0,  70,  30, 
     16,   0,   4,   0,   0,   0, 
     68,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    114,   0,  16,   0,   2,   0, 
      0,   0, 246,  15,  16,   0, 
      1,   0,   0,   0,  70,  18, 
     16,   0,   4,   0,   0,   0, 
     17,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  30,  16,   0,   5,   0, 
      0,   0,  70,  30,  16,   0, 
      5,   0,   0,   0,  68,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   3,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,  18,  16,   0, 
      5,   0,   0,   0,  69,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0, 114,   0, 
     16,   0,   4,   0,   0,   0, 
     70,  16,  16,   0,   3,   0, 
      0,   0,  70, 126,  16,   0, 
      1,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  15, 114,   0, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   4,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  64,   0,   0, 
      0,  64,   0,   0,   0,  64, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128, 191, 
      0,   0, 128, 191,   0,   0, 
    128, 191,   0,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   3,   0, 
      0,   0,  86,   5,  16,   0, 
      4,   0,   0,   0,  50,   0, 
      0,   9, 114,   0,  16,   0, 
      2,   0,   0,   0,   6,   0, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   2,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  50,   0, 
      0,   9, 114,   0,  16,   0, 
      1,   0,   0,   0, 166,  10, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      2,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     17,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  68,   0, 
      0,   5, 130,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   1,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,   2,  16,   0, 
      1,   0,   0,   0,  69,   0, 
      0, 139, 194,   0,   0, 128, 
     67,  85,  21,   0, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     70,  16,  16,   0,   3,   0, 
      0,   0,  70, 126,  16,   0, 
      2,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     49,   0,   0,   8, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,  16,  32,   0,   9, 
     18,   0,  16,   0,   3,   0, 
      0,   0,  70, 130,  32, 128, 
     65,   0,   0,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    242,   0,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,   6,   0, 
     16,   0,   3,   0,   0,   0, 
      0,   0,   0,   9, 114,   0, 
     16,   0,   4,   0,   0,   0, 
     70,  18,  16, 128,  65,   0, 
      0,   0,   1,   0,   0,   0, 
     70, 130,  32,   0,   5,   0, 
      0,   0,   8,   0,   0,   0, 
     16,   0,   0,   7, 130,   0, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  68,   0, 
      0,   5, 130,   0,  16,   0, 
      4,   0,   0,   0,  58,   0, 
     16,   0,   4,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   4,   0,   0,   0, 
    246,  15,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  16,   0, 
      0,   8, 130,   0,  16,   0, 
      4,   0,   0,   0,  70, 130, 
     32,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   7, 130,   0, 
     16,   0,   4,   0,   0,   0, 
     58,   0,  16,   0,   4,   0, 
      0,   0,  58,   0,  16,   0, 
      4,   0,   0,   0,  50,   0, 
      0,  11, 114,   0,  16,   0, 
      5,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
    246,  15,  16, 128,  65,   0, 
      0,   0,   4,   0,   0,   0, 
     70, 130,  32,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
     16,   0,   0,   7, 130,   0, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      5,   0,   0,   0,  68,   0, 
      0,   5, 130,   0,  16,   0, 
      4,   0,   0,   0,  58,   0, 
     16,   0,   4,   0,   0,   0, 
     56,   0,   0,   7, 114,   0, 
     16,   0,   5,   0,   0,   0, 
    246,  15,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      5,   0,   0,   0,  16,   0, 
      0,   7,  18,   0,  16,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   5,   0, 
      0,   0,  49,   0,   0,   7, 
     34,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   4,   0,   0,   0, 
     47,   0,   0,   6,  18,   0, 
     16,   0,   4,   0,   0,   0, 
     10,   0,  16, 128, 129,   0, 
      0,   0,   4,   0,   0,   0, 
     56,   0,   0,   7,  18,   0, 
     16,   0,   4,   0,   0,   0, 
     10,   0,  16,   0,   4,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 122,  68,  25,   0, 
      0,   5,  18,   0,  16,   0, 
      4,   0,   0,   0,  10,   0, 
     16,   0,   4,   0,   0,   0, 
     56,   0,   0,   7, 242,   0, 
     16,   0,   6,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,   6,   0,  16,   0, 
      4,   0,   0,   0,  50,   0, 
      0,  12, 242,   0,  16,   0, 
      6,   0,   0,   0,  70,  14, 
     16,   0,   6,   0,   0,   0, 
    166,  26,  16,   0,   3,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 128,  63,  55,   0, 
      0,  12, 242,   0,  16,   0, 
      4,   0,   0,   0,  86,   5, 
     16,   0,   4,   0,   0,   0, 
     70,  14,  16,   0,   6,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 128,  63,  54,  32, 
      0,   5, 242,   0,  16,   0, 
      6,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   3,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  70, 142,  32,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      6,   0,   0,   0,  56,   0, 
      0,   8, 114,   0,  16,   0, 
      3,   0,   0,   0,  70,   2, 
     16,   0,   3,   0,   0,   0, 
    246, 143,  32,   0,   2,   0, 
      0,   0,   2,   0,   0,   0, 
     55,   0,   0,  12, 242,   0, 
     16,   0,   3,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 128,  63, 
     55,   0,   0,  12, 242,   0, 
     16,   0,   4,   0,   0,   0, 
    246,  15,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 128,  63, 
      0,   0,   0,  10, 114,   0, 
     16,   0,   6,   0,   0,   0, 
    102,  24,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
    194,  38,  50,  67, 100, 185, 
    111,  65,  23,  88, 222,  66, 
      0,   0,   0,   0,  56,   0, 
      0,  10, 114,   0,  16,   0, 
      6,   0,   0,   0,  70,   2, 
     16,   0,   6,   0,   0,   0, 
      2,  64,   0,   0, 226, 138, 
    146,  60, 171,   8,  14,  61, 
    158,  52, 152,  60,   0,   0, 
      0,   0,  27,   0,   0,   5, 
    114,   0,  16,   0,   6,   0, 
      0,   0,  70,   2,  16,   0, 
      6,   0,   0,   0,  38,   0, 
      0,  11,   0, 208,   0,   0, 
     50,   0,  16,   0,   6,   0, 
      0,   0,  70,   0,  16,   0, 
      6,   0,   0,   0,   2,  64, 
      0,   0,  25,   0,   0,   0, 
      5,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     30,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      6,   0,   0,   0,  30,   0, 
      0,   7, 130,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   6,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0, 167,   0,   0, 139, 
      2, 163,   0, 128, 131, 153, 
     25,   0, 130,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   6, 112,  16,   0, 
      4,   0,   0,   0,  38,   0, 
      0,   8,   0, 208,   0,   0, 
    130,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   3,   0,   0,   0, 
     49,   0,   0,  10, 130,   0, 
     16,   0,   5,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  58, 128,  32,   6, 
      3,   0,   0,   0,   2,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  31,   0, 
      4,   3,  58,   0,  16,   0, 
      5,   0,   0,   0,   0,   0, 
      0,  10, 114,   0,  16,   0, 
      6,   0,   0,   0,  70,  18, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,  70, 130, 
     32,   4,   3,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  16,   0,   0,   7, 
    130,   0,  16,   0,   5,   0, 
      0,   0,  70,   2,  16,   0, 
      6,   0,   0,   0,  70,   2, 
     16,   0,   6,   0,   0,   0, 
      0,   0,   0,  13, 130,   0, 
     16,   0,   6,   0,   0,   0, 
     58, 128,  32,   6,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  58, 128,  32,   6, 
      3,   0,   0,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      1,   0,   0,   0,  14,   0, 
      0,   7, 130,   0,  16,   0, 
      6,   0,   0,   0,  58,   0, 
     16,   0,   5,   0,   0,   0, 
     58,   0,  16,   0,   6,   0, 
      0,   0,   0,  32,   0,   8, 
    130,   0,  16,   0,   6,   0, 
      0,   0,  58,   0,  16, 128, 
     65,   0,   0,   0,   6,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 128,  63,  68,   0, 
      0,   5,  18,   0,  16,   0, 
      7,   0,   0,   0,  58,   0, 
     16,   0,   5,   0,   0,   0, 
     56,   0,   0,   7, 226,   0, 
     16,   0,   7,   0,   0,   0, 
      6,   9,  16,   0,   6,   0, 
      0,   0,   6,   0,  16,   0, 
      7,   0,   0,   0,  50,   0, 
      0,  10,  18,   0,  16,   0, 
      8,   0,   0,   0,  10,   0, 
     16,   0,   6,   0,   0,   0, 
     10,   0,  16,   0,   7,   0, 
      0,   0,  42, 128,  32,   0, 
      5,   0,   0,   0,   0,   0, 
      0,   0,  50,   0,   0,  10, 
     34,   0,  16,   0,   8,   0, 
      0,   0,  26,   0,  16,   0, 
      6,   0,   0,   0,  10,   0, 
     16,   0,   7,   0,   0,   0, 
     42, 128,  32,   0,   5,   0, 
      0,   0,   1,   0,   0,   0, 
     50,   0,   0,  10,  66,   0, 
     16,   0,   8,   0,   0,   0, 
     42,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      7,   0,   0,   0,  42, 128, 
     32,   0,   5,   0,   0,   0, 
      2,   0,   0,   0,  16,   0, 
      0,   7,  18,   0,  16,   0, 
      6,   0,   0,   0,  70,   2, 
     16,   0,   8,   0,   0,   0, 
     70,   2,  16,   0,   8,   0, 
      0,   0,  68,   0,   0,   5, 
     18,   0,  16,   0,   6,   0, 
      0,   0,  10,   0,  16,   0, 
      6,   0,   0,   0,  56,   0, 
      0,   7, 114,   0,  16,   0, 
      6,   0,   0,   0,   6,   0, 
     16,   0,   6,   0,   0,   0, 
     70,   2,  16,   0,   8,   0, 
      0,   0,  16,  32,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0, 150,   7,  16,   0, 
      7,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     14,   0,   0,  10,  34,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0, 128,  63, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  58,   0,  16,   0, 
      5,   0,   0,   0,  56,   0, 
      0,  10,  34,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     58, 128,  32,   6,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     16,   0,   0,   7,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   6,   0, 
      0,   0,  70,   2,  16,   0, 
      5,   0,   0,   0,  49,   0, 
      0,   7,  66,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  47,   0,   0,   6, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16, 128, 
    129,   0,   0,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,  67, 
     25,   0,   0,   5,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    242,   0,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  86,   5, 
     16,   0,   1,   0,   0,   0, 
     56,   0,   0,   7, 242,   0, 
     16,   0,   2,   0,   0,   0, 
    246,  15,  16,   0,   6,   0, 
      0,   0,  70,  14,  16,   0, 
      2,   0,   0,   0,  50,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
    166,  26,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,  55,   0, 
      0,   9, 242,   0,  16,   0, 
      2,   0,   0,   0, 166,  10, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,  56,   0, 
      0,  10, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   6,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  56,   0,   0,   7, 
    242,   0,  16,   0,   0,   0, 
      0,   0, 246,  15,  16,   0, 
      6,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     56,  32,   0,   7, 242,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,  10, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
    246, 143,  32,   6,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,  24,   0,   0,   8, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  58, 128,  32,   0, 
      2,   0,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  24,   0, 
      0,  10,  34,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     58, 128,  32,   6,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   1,   0, 
      0,   0,   1,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     55,   0,   0,  12, 242,   0, 
     16,   0,   4,   0,   0,   0, 
      6,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
     18,   0,   0,   1,  54,   0, 
      0,   8, 242,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 128,  63, 
     21,   0,   0,   1,   0,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   3,   0, 
      0,   0,  54,  32,   0,   5, 
    242,   0,  16,   0,   4,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,   0,  32, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   4,   0, 
      0,   0,  49,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0, 205, 204, 204,  61, 
     13,   0,   4,   3,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5, 242,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0, 100,   0,   0,   0, 
      9,   0,   0,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     75,   0,   0,   0,   4,   0, 
      0,   0,   1,   0,   0,   0, 
      2,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      5,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
