#version 330

//layout (location = 0) in vec3 pos; // Particle initial velocity
in vec3 pos; 
//layout (location = 1) in float StartTime;    // Particle "birth" time

//out vec4 outcolor;  // Transparency of the particle

//uniform float Time;  // Animation time
//uniform vec3 Gravity = vec3(0.0,-0.05,0.0);  // world coords
//uniform float ParticleLifetime;  // Max particle lifetime

in vec2 s_vTexCoord;    // Coming FROM the OpenGL program
out vec2 texCoord;      // Going TO the fragment shader


uniform float LifeTime;
uniform mat4 MVP;

out float v_ageFactor;

void main()
{
    // Assume the initial position is (0,0,0).
    //vec3 pos = vec3(0.0);
    //Transp = 0.0;

    v_ageFactor = (LifeTime+5000)/(10000.0+5000);
    
    texCoord = s_vTexCoord;

    // Draw at the current position
    gl_Position = MVP*vec4(pos, 1.0);
    gl_PointSize = 40.0; // size in pixels
    //outcolor=vec4(0.0,1.0,0.0,1.0);
}
