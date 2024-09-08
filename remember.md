D3D12 Mesh Shaders
- Pipeline States
D3D12MeshletRender.cpp L.260
D3D12RootSignature
- Objects in CPU mode must be set in the order declared on the RSig
-- Example: `#define rs_controller \
RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), \
RootConstants(num32BitConstants=48, b0), \
CBV(b1), \
SRV(t0), \
DescriptorTable(Sampler(s0, numDescriptors = 1)) \`, the first to be set must be the root constants, the second the CBV, the third, SRV and the fourth, the descriptor table with the sample

D3D12 Sampler
- The root signature will only accept dynamic samplers inside a Descriptor Table
- The samplers aren't allowed to be bound, only set and reset

VK Texture
- All textures must be in pair with a sampler