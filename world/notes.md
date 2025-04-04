# World - the one you want to raytrace

This file explains how to edit the world used by the raytracer to make (nowhere near) anything you like.  
All of the data is in easy-to-read and understand JSON files.

---

## materials.json
This file describes all the materials that objects in your scene can use.

- **`type`**: Defines the type of material:
  - `0`: Lambertian (diffuse surface).
  - `1`: Metal (reflective surface).
  - `2`: Dielectric (transparent/refractive surface, e.g., glass).
  - `3`: Emissive (light-emitting surface).
- **`albedo`**: The proportion of light (in RGB) that the material reflects, scaled from `0.0` to `1.0` for each color.
- **`fuzz`**: (Only for metal) Determines how "blurry" the reflections are. A value of `0.0` is perfectly smooth, while higher values make reflections more scattered.
- **`refractionIndex`**: (Only for dielectric) Defines the index of refraction for the material (e.g., `1.5` for glass).
- **`texture`**: (Optional) A texture file to define the color reflected instead of the albedo. Use the file name inside the `world` folder (e.g., `"texture.png"`). Set `albedo` to `[0.0, 0.0, 0.0]` if using a texture.

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