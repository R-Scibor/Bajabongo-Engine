#version 110

uniform sampler2D sceneTexture;
uniform sampler2D sightMap;
uniform sampler2D explorationMap;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    
    // Sample all three textures
    vec4 sceneColor = texture2D(sceneTexture, uv);
    float sightValue = texture2D(sightMap, uv).r;
    float explorationValue = texture2D(explorationMap, uv).r;
    
    // Three-layer fog logic
    if (sightValue > 0.5) {
        // Fully visible - current sight area
        gl_FragColor = sceneColor;
    } else if (explorationValue > 0.5) {
        // Explored but not visible - memory (greyed out)
        gl_FragColor = vec4(sceneColor.rgb * 0.5, sceneColor.a);
    } else {
        // Unexplored - pure black
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
