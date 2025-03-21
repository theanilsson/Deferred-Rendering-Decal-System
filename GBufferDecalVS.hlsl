#include "Common.hlsli"

ModelVertexToPixel main(ModelVertexInput input)
{
    ModelVertexToPixel result;

    float4 vertexLocalSpacePosition = input.position;
    float4 vertexWorldSpacePosition = mul(ObjectToWorld, vertexLocalSpacePosition);
    float4 vertexCameraSpacePosition = mul(WorldToCamera, vertexWorldSpacePosition);
    result.position = mul(CameraToProjection, vertexCameraSpacePosition);
	    
    return result;
}