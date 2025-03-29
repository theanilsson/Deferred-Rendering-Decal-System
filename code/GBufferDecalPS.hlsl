#include "common.hlsli"

PixelOutput main(ModelVertexToPixel input)
{
    PixelOutput output;
    
    float2 geometryBufferUV = input.position.xy / Resolution.zw;
    float4 worldSpacePosition = gBufferPositionTexture.Sample(defaultSampler, geometryBufferUV).rgba;
    float4 localSpacePosition = mul(worldSpacePosition, ObjectToWorldInverse);
    clip(0.5 - abs(localSpacePosition.xyz));
    
    float2 decalTextureCoordinate = localSpacePosition.xy + 0.5;
    output.color = decalTexture.SampleLevel(defaultSampler, decalTextureCoordinate, 0);
    
    return output;
}