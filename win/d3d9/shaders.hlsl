
struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain2(float4 position : POSITION, float4 color : COLOR)
{

	PSInput result;

	result.position = float4(0.0,0.0,0.0,0.0);
	result.color = float4(1.0,0.0, 0.0,1.0);

	return result;
}
float4 VSMain(float4 position : POSITION, float4 color : COLOR) : POSITION
{
	return float4(1.0,0.0,0.0,1.0);
}
float4 PSMain(PSInput input) : SV_TARGET
{
   //return float4(1.0,0.0, 1.0,1.0);
	return input.color;
}
