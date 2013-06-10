#version 150

uniform sampler2D gnormals;
uniform sampler2D gdepth;
uniform sampler2D gdiffuse;
uniform sampler2D grandom;

uniform vec2 LinMAD;

in vec2 TexCoord;

vec3 ReadNormal(in vec2 coord)  
{  
    vec3 N = vec3(texture(texture1, coord).xy, 0.0);
    N.z = sqrt(1.0 - dot(N.xy, N.xy));
    return N;
}

float ViewSpaceZFromDepth(float d)
{
	// [0,1] -> [-1,1] clip space
	d = d * 2.0 - 1.0;

	// Get view space Z
	return -1.0 / (LinMAD.x * d + LinMAD.y);
}

vec3 UVToViewSpace(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return vec3(uv * z, z);
}

vec3 GetViewPosFront(vec2 uv)
{
	float z = ViewSpaceZFromDepth(texture(texture0, uv).r);
	return UVToViewSpace(uv, z);
}

//Ambient Occlusion form factor:
float aoFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2){
      vec3 vv = normalize(ddiff);
      float rd = length(ddiff);
      return (1.0-clamp(dot(readNormal(TexCoord+vec2(c1,c2)),-vv),0.0,1.0)) *
       clamp(dot( cnorm,vv ),0.0,1.0)* 
             (1.0 - 1.0/sqrt(1.0/(rd*rd) + 1.0));
}

//GI form factor:
float giFF(in vec3 ddiff,in vec3 cnorm, in float c1, in float c2){
      vec3 vv = normalize(ddiff);
      float rd = length(ddiff);
      return 1.0*clamp(dot(readNormal(TexCoord+vec2(c1,c2)),-vv),0.0,1.0)*
                 clamp(dot( cnorm,vv ),0.0,1.0)/
                 (rd*rd+1.0);  
}

void main()
{
    //read current normal,position and color.
    vec3 n = readNormal(TexCoord);
    vec3 p = posFromDepth(TexCoord);
    vec3 col = texture2D(gdiffuse, TexCoord).rgb;

    //randomization texture
    vec2 fres = vec2(800.0/128.0*5,600.0/128.0*5);
    vec3 random = texture2D(grandom, TexCoord*fres.xy);
    random = random*2.0-vec3(1.0);

    //initialize variables:
    float ao = 0.0;
    vec3 gi = vec3(0.0,0.0,0.0);
    float incx = 1.0/800.0*0.1;
    float incy = 1.0/600.0*0.1;
    float pw = incx;
    float ph = incy;
    float cdepth = texture2D(gdepth, TexCoord).r;

    //3 rounds of 8 samples each. 
    for(float i=0.0; i<3.0; ++i) 
    {
       float npw = (pw+0.0007*random.x)/cdepth;
       float nph = (ph+0.0007*random.y)/cdepth;

       vec3 ddiff = posFromDepth(TexCoord.st+vec2(npw,nph))-p;
       vec3 ddiff2 = posFromDepth(TexCoord.st+vec2(npw,-nph))-p;
       vec3 ddiff3 = posFromDepth(TexCoord.st+vec2(-npw,nph))-p;
       vec3 ddiff4 = posFromDepth(TexCoord.st+vec2(-npw,-nph))-p;
       vec3 ddiff5 = posFromDepth(TexCoord.st+vec2(0,nph))-p;
       vec3 ddiff6 = posFromDepth(TexCoord.st+vec2(0,-nph))-p;
       vec3 ddiff7 = posFromDepth(TexCoord.st+vec2(npw,0))-p;
       vec3 ddiff8 = posFromDepth(TexCoord.st+vec2(-npw,0))-p;

       ao+=  aoFF(ddiff,n,npw,nph);
       ao+=  aoFF(ddiff2,n,npw,-nph);
       ao+=  aoFF(ddiff3,n,-npw,nph);
       ao+=  aoFF(ddiff4,n,-npw,-nph);
       ao+=  aoFF(ddiff5,n,0,nph);
       ao+=  aoFF(ddiff6,n,0,-nph);
       ao+=  aoFF(ddiff7,n,npw,0);
       ao+=  aoFF(ddiff8,n,-npw,0);

       gi+=  giFF(ddiff,n,npw,nph)*texture2D(gdiffuse, TexCoord+vec2(npw,nph)).rgb;
       gi+=  giFF(ddiff2,n,npw,-nph)*texture2D(gdiffuse, TexCoord+vec2(npw,-nph)).rgb;
       gi+=  giFF(ddiff3,n,-npw,nph)*texture2D(gdiffuse, TexCoord+vec2(-npw,nph)).rgb;
       gi+=  giFF(ddiff4,n,-npw,-nph)*texture2D(gdiffuse, TexCoord+vec2(-npw,-nph)).rgb;
       gi+=  giFF(ddiff5,n,0,nph)*texture2D(gdiffuse, TexCoord+vec2(0,nph)).rgb;
       gi+=  giFF(ddiff6,n,0,-nph)*texture2D(gdiffuse, TexCoord+vec2(0,-nph)).rgb;
       gi+=  giFF(ddiff7,n,npw,0)*texture2D(gdiffuse, TexCoord+vec2(npw,0)).rgb;
       gi+=  giFF(ddiff8,n,-npw,0)*texture2D(gdiffuse, TexCoord+vec2(-npw,0)).rgb;

       //increase sampling area:
       pw += incx;  
       ph += incy;    
    } 
    ao/=24.0;
    gi/=24.0;


    gl_FragColor = vec4(col-vec3(ao)+gi*5.0,1.0);
}