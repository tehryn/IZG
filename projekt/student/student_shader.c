/*!
 * @file
 * @brief This file contains implemenation of phong vertex and fragment shader.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include<math.h>
#include<assert.h>

#include"student/student_shader.h"
#include"student/gpu.h"
#include"student/uniforms.h"

/// \addtogroup shader_side Úkoly v shaderech
/// @{

void phong_vertexShader(
    GPUVertexShaderOutput     *const output,
    GPUVertexShaderInput const*const input ,
    GPU                        const gpu   ){
    /// \todo Naimplementujte vertex shader, který transformuje vstupní vrcholy do clip-space.<br>
    /// <b>Vstupy:</b><br>
    /// Vstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu ve world-space (vec3) a v prvním
    /// atributu obsahovat normálu vrcholu ve world-space (vec3).<br>
    /// <b>Výstupy:</b><br>
    /// Výstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu (vec3) ve world-space a v prvním
    /// atributu obsahovat normálu vrcholu ve world-space (vec3).
    /// Výstupní vrchol obsahuje pozici a normálu vrcholu proto, že chceme počítat osvětlení ve world-space ve fragment shaderu.<br>
    /// <b>Uniformy:</b><br>
    /// Vertex shader by měl pro transformaci využít uniformní proměnné obsahující view a projekční matici.
    /// View matici čtěte z uniformní proměnné "viewMatrix" a projekční matici čtěte z uniformní proměnné "projectionMatrix".
    /// Zachovejte jména uniformních proměnných a pozice vstupních a výstupních atributů.
    /// Pokud tak neučiníte, akceptační testy selžou.<br>
    /// <br>
    /// Využijte vektorové a maticové funkce.
    /// Nepředávajte si data do shaderu pomocí globálních proměnných.
    /// Pro získání dat atributů použijte příslušné funkce vs_interpret* definované v souboru program.h.
    /// Pro získání dat uniformních proměnných použijte příslušné funkce shader_interpretUniform* definované v souboru program.h.
    /// Vrchol v clip-space by měl být zapsán do proměnné gl_Position ve výstupní struktuře.<br>
    /// <b>Seznam funkcí, které jistě použijete</b>:
    ///  - gpu_getUniformsHandle()
    ///  - getUniformLocation()
    ///  - shader_interpretUniformAsMat4()
    ///  - vs_interpretInputVertexAttributeAsVec3()
    ///  - vs_interpretOutputVertexAttributeAsVec3()
    Uniforms const uniformsHandle = gpu_getUniformsHandle(gpu);
    UniformLocation const viewMatrixLocation = getUniformLocation(gpu, "viewMatrix");
    Mat4 const*const view = shader_interpretUniformAsMat4(uniformsHandle, viewMatrixLocation);
    UniformLocation const projectionMatrixLocation = getUniformLocation(gpu, "projectionMatrix");
    Mat4 const*const proj = shader_interpretUniformAsMat4(uniformsHandle, projectionMatrixLocation);
    Vec3 const*const position = vs_interpretInputVertexAttributeAsVec3(gpu, input, 0);
    Vec3 const*const normalPosition = vs_interpretInputVertexAttributeAsVec3(gpu, input, 1);

    Mat4 mvp;
    multiply_Mat4_Mat4(&mvp,proj,view);

    Vec4 pos4;
    copy_Vec3Float_To_Vec4(&pos4,position,1.f);
    multiply_Mat4_Vec4(&output->gl_Position, &mvp, &pos4);

    Vec3 *const fragment = vs_interpretOutputVertexAttributeAsVec3(gpu, output, 0);
    Vec3 *const normal = vs_interpretOutputVertexAttributeAsVec3(gpu, output, 1);

    fragment->data[0] = position->data[0];
    fragment->data[1] = position->data[1];
    fragment->data[2] = position->data[2];
    normal->data[0]   = normalPosition->data[0];
    normal->data[1]   = normalPosition->data[1];
    normal->data[2]   = normalPosition->data[2];
}

#define MAX(x, y) ((x)>(y)?(x):(y))
void phong_fragmentShader(
    GPUFragmentShaderOutput     *const output,
    GPUFragmentShaderInput const*const input ,
    GPU                          const gpu   ){
    /// \todo Naimplementujte fragment shader, který počítá phongův osvětlovací model s phongovým stínováním.<br>
    /// <b>Vstup:</b><br>
    /// Vstupní fragment by měl v nultém fragment atributu obsahovat interpolovanou pozici ve world-space a v prvním
    /// fragment atributu obsahovat interpolovanou normálu ve world-space.<br>
    /// <b>Výstup:</b><br>
    /// Barvu zapište do proměnné color ve výstupní struktuře.<br>
    /// <b>Uniformy:</b><br>
    /// Pozici kamery přečtěte z uniformní proměnné "cameraPosition" a pozici světla přečtěte z uniformní proměnné "lightPosition".
    /// Zachovejte jména uniformních proměnný.
    /// Pokud tak neučiníte, akceptační testy selžou.<br>
    /// <br>
    /// Dejte si pozor na velikost normálového vektoru, při lineární interpolaci v rasterizaci může dojít ke zkrácení.
    /// Zapište barvu do proměnné color ve výstupní struktuře.
    /// Shininess faktor nastavte na 40.f
    /// Difuzní barvu materiálu nastavte na čistou zelenou.
    /// Spekulární barvu materiálu nastavte na čistou bílou.
    /// Barvu světla nastavte na bílou.
    /// Nepoužívejte ambientní světlo.<br>
    /// <b>Seznam funkcí, které jistě využijete</b>:
    ///  - shader_interpretUniformAsVec3()
    ///  - fs_interpretInputAttributeAsVec3()
    Uniforms handler     = gpu_getUniformsHandle(gpu);
    const Vec3 *camera   = shader_interpretUniformAsVec3(handler, getUniformLocation(gpu, "cameraPosition"));
    const Vec3 *light    = shader_interpretUniformAsVec3(handler, getUniformLocation(gpu, "lightPosition"));
    const Vec3 *fragment = fs_interpretInputAttributeAsVec3(gpu, input, 0);
    const Vec3 *fragment_normal  = fs_interpretInputAttributeAsVec3(gpu, input, 1);

    Vec3 normal;
    normalize_Vec3(&normal, fragment_normal);

    Vec3 Lvector;
    sub_Vec3(&Lvector, light, fragment);
    normalize_Vec3(&Lvector, &Lvector);

    Vec3 Vvector;
    sub_Vec3(&Vvector, camera, fragment);
    normalize_Vec3(&Vvector, &Vvector);

    float r = 0.0, g = 0.0, b = 0.0;
    float NL = dot_Vec3(&normal, &Lvector);
    if (NL > 0) {
        g += NL;
    }

    Vec3 Rvector;
    multiply_Vec3_Float(&Rvector, &normal, 2*NL);
    sub_Vec3(&Rvector, &Rvector, &Lvector);

    float RV  = powf(MAX(dot_Vec3(&Rvector, &Vvector), 0.0f), 40.0f);
    if (RV > 0) {
        r += RV;
        g += RV;
        b += RV;
    }
    output->color.data[0] = r;
    output->color.data[1] = g;
    output->color.data[2] = b;
    output->color.data[3] = 1.0f;
}

/// @}
