# Raytracer

A real-time raytracer built using **Raylib** and **OpenGL 3.3 Core**.

## Features
- Ray tracing with basic primitives (spheres, quads, and more to come later)
- Realistic lighting model
- Adjustable camera and scene controls
- Optimized rendering pipeline using OpenGL shaders

## Setup
- Simply running the exe will start the raytracer
- It gathers the information about the scene by looking in the `world` folder
- There is more information provided in the `world` folder telling you how to edit it


## Controls
- `WASD` for moving around
- `EQ` for up-down movement
- `P` to pause and open the pause settings
- `T` to toggle fullscreen
- `H` to render a high quality image (only when the settings menu isn't open)

---

This is just a project of myn I'm working on in my free time.\
My main resource for learning is from [Ray Tracing in One Weekend Book Series](https://raytracing.github.io/)\
And thanks to everyone who has worked on Raylib\
I hope you enjoy messing around with my raytracer.



---



# The World

This following explains how to edit the world used by the raytracer to make (nowhere near) anything you like.  
All of the data is in easy-to-read and understand JSON files.

---

## materials.json
This file describes all the materials that objects in your scene can use.

- **`type`**: Defines the type of material:
  - `0`: Lambertian (diffuse surface).
  - `1`: Metal (reflective surface).
  - `2`: Dielectric (transparent/refractive surface, e.g., glass).
- **`albedo`**: The proportion of light (in RGB) that the material reflects, should be between `0.0` to `1.0` for each color.
- **`emmisiveColor`**: (Optional) The light (in RGB) that a material will emit. Works on all types of materials.
- **`fuzz`**: (Only for metal) Determines how "blurry" the reflections are. A value of `0.0` is perfectly smooth, while higher values make reflections more scattered.
- **`refractionIndex`**: (Only for dielectric) Defines the index of refraction for the material (e.g., `1.5` for glass).
- **`texture`**: (Optional) A texture file to define the color reflected instead of the albedo. Use the file name inside the `world` folder (e.g., `"texture.png"`).

---

## spheres.json
This file defines all the spheres in the scene.

- **`position`**: The center of the sphere in 3D space, given as `[x, y, z]`.
- **`radius`**: The radius of the sphere.
- **`materialIndex`**: The index of the material to apply to the sphere. This corresponds to the order of materials in `materials.json`.

---

## quads.json
This file defines all the quads (rectangles) in the scene.

- **`origin`**: The starting point (corner) of the quad in 3D space, given as `[x, y, z]`.
- **`edgeU`**: A vector defining one edge of the quad, starting from the origin.
- **`edgeV`**: A vector defining the other edge of the quad, starting from the origin. Together with `edgeU`, this defines the plane of the quad.
- **`materialIndex`**: The index of the material to apply to the quad. This corresponds to the order of materials in `materials.json`.

---

## Notes
- You can store multiple world folders at a time, but the folder named `world` will be the one that is used for rendering.
- You don't have to fill out all the data, but everything will default to zero. This is useful in materials where `fuzz` and `refractionIndex` are only used for certain types of materials.
