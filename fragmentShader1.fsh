#version 330

//in vec4 outcolor;
out vec4 FragColor;
//uniform sampler2D ParticleTex;

//layout ( location = 0 ) out vec4 FragColor;

in float v_ageFactor;
in vec2 texCoord;				
uniform sampler2D texture;

void main()
{
	vec4 texColour;
    vec2 texCoords;
    float alphaFactor;

    texCoords = vec2(gl_PointCoord.x, gl_PointCoord.y);

    if(v_ageFactor <= 0.5)
     {
     alphaFactor = 0.08* v_ageFactor;
    //alphaFactor =0;
     }

     else
     {
     alphaFactor = -0.8* v_ageFactor;
     //alphaFactor =0;
     }


    texColour = texture2D(texture, texCoords);

    texColour.a = texColour.r * alphaFactor;


	//vec4 texel = texture2D(texture, texCoord);
    
    FragColor = vec4(texColour.r, texColour.g, texColour.b, texColour.a); 
   
}
