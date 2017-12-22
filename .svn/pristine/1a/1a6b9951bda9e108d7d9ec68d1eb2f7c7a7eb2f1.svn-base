
#version 330

in vec4 vertex;
in vec3 normal;
in vec2 st;

out vec4 fragColour;

uniform vec4 light_pos;

uniform vec3 light_ambient;     // Light ambient RGBA values
uniform vec3 light_diffuse;     // Light diffuse RGBA values
uniform vec3 light_specular;    // Light specular RGBA values

uniform vec3 mtl_ambient;  // Ambient surface colour

const float shininess = 8;

uniform sampler2D diffMap;
uniform sampler2D specMap;


// The Phong lighting calculation for this fragment.
// Note use of texture lookups for diffuse and specular colour.
vec3 phongPointLight(in vec4 position, in vec3 norm)
{
		vec4 a = light_pos;
		vec3 s = normalize(vec3(light_pos - position));
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect( -s, norm );

    vec3 ambient = light_ambient * mtl_ambient;

    // The diffuse component
    float sDotN = max( dot(s,norm), 0.0 );
    vec3 diffuse = light_diffuse * vec3(texture(diffMap, st)) * sDotN;

    // The specular component
    vec3 spec = vec3(0.0);
    if ( sDotN > 0.0 )
		spec = light_specular * vec3(texture(specMap, st)) *
            pow( max( dot(r,v), 0.0 ), shininess );

    return ambient + diffuse + spec;
}

void main(void)
{
	if (length(vertex) <= 1.5){
		vec3 mixture = (vec3(texture(diffMap, st)) + vec3(texture(specMap, st)))/2.0;
		fragColour = vec4(mixture, 1.0);
	}else{
		fragColour = vec4(phongPointLight(vertex, normalize(normal)), 1.0);
	}
}

/*

#version 330

in vec4 vertex;
in vec3 normal;
in vec2 st;

out vec4 fragColour;

uniform vec4 light_pos;

//uniform vec3 light_ambient;     // Light ambient RGBA values
//uniform vec3 light_diffuse;     // Light diffuse RGBA values
//uniform vec3 light_specular;    // Light specular RGBA values

//uniform vec3 mtl_ambient;  // Ambient surface colour

const float shininess = 8;

uniform sampler2D diffMap;
uniform sampler2D specMap;

// The Phong lighting calculation for this fragment.
// Note use of texture lookups for diffuse and specular colour.
vec3 phongPointLight(in vec4 position, in vec3 norm)
{
    vec3 s = normalize(vec3(light_pos - position));
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect( -s, norm );

    vec3 ambient = light_ambient * mtl_ambient;

    // The diffuse component
    float sDotN = max( dot(s,norm), 0.0 );
    vec3 diffuse = light_diffuse * vec3(texture(diffMap, st)) * sDotN;

    // The specular component
    vec3 spec = vec3(0.0);
    if ( sDotN > 0.0 )
		spec = light_specular * vec3(texture(specMap, st)) *
            pow( max( dot(r,v), 0.0 ), shininess );

    return ambient + diffuse + spec;
}

void main(void)
{
	vec4 lol = vec4(phongPointLight(vertex, normalize(normal)), 1.0);
	fragColour = vec4(vec3(texture(diffMap, st)), 1.0);
}*/
