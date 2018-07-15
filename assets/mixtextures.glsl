uniform vec3      	iResolution; 			// viewport resolution (in pixels)
uniform vec3        iChannelResolution[4];	// channel resolution (in pixels)
uniform float     	iGlobalTime; 			// shader playback time (in seconds)
uniform vec4      	iMouse; 				// mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D 	iChannel0; 				// input channel 0
uniform sampler2D 	iChannel1; 				// input channel 0

out vec4 fragColor;
vec2  fragCoord = gl_FragCoord.xy; // keep the 2 spaces between vec2 and fragCoord
// Blend functions begin
vec3 multiply( vec3 s, vec3 d )
{
   return s*d;
}
vec3 colorBurn( vec3 s, vec3 d )
{
   return 1.0 - (1.0 - d) / s;
}
vec3 linearBurn( vec3 s, vec3 d )
{
   return s + d - 1.0;
}
vec3 darkerColor( vec3 s, vec3 d )
{
   return (s.x + s.y + s.z < d.x + d.y + d.z) ? s : d;
}
vec3 lighten( vec3 s, vec3 d )
{
   return max(s,d);
}
vec3 darken( vec3 s, vec3 d )
{
   return min(s,d);
}
vec3 screen( vec3 s, vec3 d )
{
   return s + d - s * d;
}

vec3 colorDodge( vec3 s, vec3 d )
{
   return d / (1.0 - s);
}

vec3 linearDodge( vec3 s, vec3 d )
{
   return s + d;
}

vec3 lighterColor( vec3 s, vec3 d )
{
   return (s.x + s.y + s.z > d.x + d.y + d.z) ? s : d;
}

float overlay( float s, float d )
{
   return (d < 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
}

vec3 overlay( vec3 s, vec3 d )
{
   vec3 c;
   c.x = overlay(s.x,d.x);
   c.y = overlay(s.y,d.y);
   c.z = overlay(s.z,d.z);
   return c;
}

float softLight( float s, float d )
{
   return (s < 0.5) ? d - (1.0 - 2.0 * s) * d * (1.0 - d) 
      : (d < 0.25) ? d + (2.0 * s - 1.0) * d * ((16.0 * d - 12.0) * d + 3.0) 
                : d + (2.0 * s - 1.0) * (sqrt(d) - d);
}

vec3 softLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = softLight(s.x,d.x);
   c.y = softLight(s.y,d.y);
   c.z = softLight(s.z,d.z);
   return c;
}

float hardLight( float s, float d )
{
   return (s < 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
}

vec3 hardLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = hardLight(s.x,d.x);
   c.y = hardLight(s.y,d.y);
   c.z = hardLight(s.z,d.z);
   return c;
}

float vividLight( float s, float d )
{
   return (s < 0.5) ? 1.0 - (1.0 - d) / (2.0 * s) : d / (2.0 * (1.0 - s));
}

vec3 vividLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = vividLight(s.x,d.x);
   c.y = vividLight(s.y,d.y);
   c.z = vividLight(s.z,d.z);
   return c;
}

vec3 linearLight( vec3 s, vec3 d )
{
   return 2.0 * s + d - 1.0;
}

float pinLight( float s, float d )
{
   return (2.0 * s - 1.0 > d) ? 2.0 * s - 1.0 : (s < 0.5 * d) ? 2.0 * s : d;
}

vec3 pinLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = pinLight(s.x,d.x);
   c.y = pinLight(s.y,d.y);
   c.z = pinLight(s.z,d.z);
   return c;
}

vec3 hardMix( vec3 s, vec3 d )
{
   return floor(s + d);
}

vec3 difference( vec3 s, vec3 d )
{
   return abs(d - s);
}

vec3 exclusion( vec3 s, vec3 d )
{
   return s + d - 2.0 * s * d;
}

vec3 subtract( vec3 s, vec3 d )
{
   return s - d;
}

vec3 divide( vec3 s, vec3 d )
{
   return s / d;
}
void main() {
	vec2 uv = 2 * (gl_FragCoord.xy / iResolution.xy);

	vec3 t0 = texture(iChannel0, uv ).xyz;
	vec3 t1 = texture(iChannel1, uv  ).xyz;
   	fragColor = vec4(vec3( multiply(t0,t1) ),1.0);
}
