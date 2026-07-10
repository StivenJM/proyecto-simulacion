# Guía de instalación del proyecto en Windows (VS Code + MSVC + CMake + vcpkg)

Esta guía permite configurar desde cero el entorno necesario para compilar y ejecutar el simulador acústico 3D utilizando:

* VS Code
* Visual Studio Build Tools
* MSVC (compilador de Microsoft)
* CMake
* vcpkg
* GLFW
* GLAD
* Dear ImGui

---

# 1. Dependencias

Instalar los siguientes componentes:

## Herramientas principales

* VS Code
* Visual Studio Build Tools

  * Seleccionar **Desarrollo para escritorio con C++**
* CMake
* Git

## Librerías

Se instalarán mediante vcpkg:

* GLFW
* GLAD
* Dear ImGui con bindings para GLFW y OpenGL 3

## Extensiones de VS Code

* C/C++
* CMake Tools

---

# 2. Verificar la instalación de Visual Studio Build Tools

MSVC no funciona directamente desde PowerShell.

Abrir una consola **CMD** y ejecutar:

```cmd
"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"
```

> Nota: la ruta puede variar dependiendo de la versión instalada.

La salida debería ser similar a:

```text
**********************************************************************
** Visual Studio 2026 Developer Command Prompt v18.7.1
** Copyright (c) 2026 Microsoft Corporation
**********************************************************************
```

Comprobar que el compilador está disponible:

```cmd
cl
```

La salida debería contener algo similar a:

```text
Microsoft (R) C/C++ Optimizing Compiler Version ...
```

---

# 3. Instalar y verificar CMake

Instalar CMake utilizando el instalador oficial para Windows.

Comprobar la instalación:

```bash
cmake --version
```

---

# 4. Instalar y verificar Git

Instalar Git utilizando el instalador oficial para Windows.

Comprobar la instalación:

```bash
git --version
```

---

# 5. Instalar vcpkg

Clonar el repositorio:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
```

Generar el ejecutable:

```powershell
bootstrap-vcpkg.bat
```

Instalar las dependencias necesarias para OpenGL e interfaz gráfica:

```powershell
.\vcpkg.exe install glfw3:x64-windows glad:x64-windows "imgui[glfw-binding,opengl3-binding]:x64-windows"
```

Verificar que quedaron instaladas:

```powershell
.\vcpkg.exe list
```

La salida debe incluir, al menos:

```text
glfw3:x64-windows
glad:x64-windows
imgui:x64-windows
```

---

# 6. Configuración de VS Code

## Habilitar MSVC dentro de VS Code

Para utilizar el compilador MSVC directamente desde la terminal integrada de VS Code, crear un perfil personalizado.

### Pasos

1. Presionar `Ctrl + Shift + P`
2. Seleccionar **Preferences: Open Settings (UI)**
3. Buscar:

```text
terminal.integrated.profiles.windows
```

4. Agregar la siguiente configuración:

```json
{
  "terminal.integrated.profiles.windows": {
    "MSVC": {
      "path": "C:\\Windows\\System32\\cmd.exe",
      "args": [
        "/k",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\Common7\\Tools\\VsDevCmd.bat",
        "-arch=x64"
      ]
    }
  }
}
```

5. Abrir una nueva terminal y seleccionar el perfil **MSVC**.

---

# 7. Configuración CMake del proyecto

## Estructura principal esperada

```text
Proyecto/
├─ CMakeLists.txt
└─ src/
   ├─ main.cpp
   ├─ core/
   └─ gui/
```

---

## Organización de build

El proyecto separa el Core de la GUI en CMake:

| Target | Rol |
|---|---|
| `simulation_core` | Librería estática interna con la simulación acústica. No depende de OpenGL. |
| `AcousticSimulator` | Ejecutable principal con GUI, rendering e integración hacia el Core. |

`AcousticSimulator` enlaza `simulation_core` junto con GLFW, GLAD, Dear ImGui y OpenGL. Esta separación permite validar el Core sin abrir ventana y mantiene los cálculos fuera de la capa gráfica.

---

## Archivo `main.cpp`

El punto de entrada del proyecto debe crear la aplicación GUI y ejecutar el loop principal. El detalle de ventana, OpenGL e ImGui vive dentro de la capa GUI del proyecto.

---

# 8. Compilación manual

Configurar el proyecto:

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Compilar:

```bash
cmake --build build
```

o

```bash
cmake --build build --config Release
```

---

## Ejecutables generados

Modo Debug:

```text
.\build\Debug\AcousticSimulator.exe
```

Modo Release:

```text
.\build\Release\AcousticSimulator.exe
```

---

# 9. Automatizar CMake en VS Code

Para evitar escribir los comandos de configuración manualmente en cada proyecto:

Crear:

```text
.vscode/settings.json
```

Contenido:

```json
{
  "cmake.configureArgs": [
    "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  ]
}
```

---

## Configurar el Kit de compilación

1. Presionar `Ctrl + Shift + P`
2. Ejecutar:

```text
CMake: Select a Kit
```

3. Seleccionar:

```text
Visual Studio Build Tools 2026 - amd64
```

4. Ejecutar:

```text
CMake: Configure
```

---

# 10. Flujo de trabajo final

Una vez configurado:

```text
Abrir VS Code
↓
Presionar Build o Run
↓
CMake configura automáticamente
↓
CMake compila automáticamente
↓
Ejecutar aplicación
```

Ya no es necesario ejecutar manualmente:

```bash
cmake -B build -S ...
cmake --build build
```

Los botones de la barra inferior de CMake Tools permiten:

* Build
* Run
* Debug

directamente desde VS Code.
