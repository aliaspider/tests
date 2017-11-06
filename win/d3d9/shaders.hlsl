
struct PSInput
{
	float4 position : POSITION;
	float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{

	PSInput result;

	result.position = position;
	result.color = color;

	return result;
}
float4 VSMain2() : POSITION
{
	return float4(1.0,0.0,0.0,1.0);
}
float4 PSMain(float4 color : COLOR) : COLOR
{
   return float4(1.0,0.0, 1.0,1.0);
	return color;
}
