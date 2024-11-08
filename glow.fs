#version 330 core

out vec4 fragColor;
in vec2 fragCoord;

uniform vec2 resolution;
uniform float radius;

void main()
{
    // Normalize the pixel coordinates to [0, 1]
    vec2 uv = fragCoord / resolution.xy;
    uv -= 0.5;
    uv.x *= resolution.x / resolution.y;

    // Calculate the distance from the center of the circle
    float d = length(uv) - radius;

    // Create the circle with a glow effect
    vec3 col = vec3(step(0.0, -d)); // White circle with black background

    // Apply the glow effect (diminishing with distance)
    float glow = 0.01 / d; // Create the glow based on the distance from the circle
    glow = clamp(glow, 0.0, 1.0); // Clamp to avoid artifacts
    col += glow * 20.; // Add the glow effect to the circle

    // Output the final color
    fragColor = vec4(col, 1.0);
}
