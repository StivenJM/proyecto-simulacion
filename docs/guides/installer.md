# Generar el instalador de Windows

Esta guía explica cómo generar el instalador `.exe` distribuible de `AcousticSimulator` usando CPack. El instalador se genera desde el target `AcousticSimulator`, por lo que incluye los cambios actuales de la aplicación cuando primero recompilás en `Release`. El resultado queda en `build/packages` y no requiere subir binarios generados al repositorio.

## Prerrequisito

Antes de seguir esta guía, completa primero `docs/guides/installation.md`.

Esa guía deja listo el entorno base necesario para compilar el proyecto:

* Visual Studio Build Tools con MSVC
* CMake
* Git
* vcpkg
* GLFW, GLAD e ImGui instalados con vcpkg
* configuración de CMake con el toolchain de vcpkg

Para generar el instalador también necesitas tener instalado NSIS. CPack puede encontrar NSIS desde su instalación de Windows.

Verifica NSIS con una de estas opciones:

```powershell
makensis /VERSION
```

## Archivos versionados necesarios

El instalador se genera desde archivos del repositorio, no desde archivos manuales externos.

| Archivo | Rol |
|---|---|
| `CMakeLists.txt` | Define el ejecutable como aplicación gráfica de Windows, las reglas `install()` y la configuración de CPack. |
| `src/` | Contiene el código fuente de la aplicación. |
| `src/main.cpp` | Define los entry points `main` y `WinMain` para que Windows ejecute la app sin abrir una consola. |
| `src/app.rc` y `src/resource.h` | Registran el icono nativo de Windows dentro del ejecutable. |
| `assets/icons/acoustic-simulator.ico` | Icono usado por el `.exe`, el acceso directo y la ventana de la aplicación. |
| `docs/guides/installation.md` | Explica cómo preparar el entorno base para compilar. |
| `docs/guides/installer.md` | Explica cómo generar el paquete distribuible. |

No se deben versionar `build/`, `build/packages/`, `_CPack_Packages/` ni instaladores generados. Esos artefactos se reconstruyen localmente.

Los tests de desarrollo no forman parte del instalador. El proyecto los mantiene detrás de `ACOUSTIC_BUILD_TESTS`, que está desactivado por defecto para que el paquete distribuible solo incluya la aplicación y sus dependencias de ejecución.

## Generación rápida

Ejecuta estos comandos desde la raíz del proyecto:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release --target AcousticSimulator
cpack --config build/CPackConfig.cmake -C Release -G NSIS64
```

Si ya tenés un `build` configurado, igual recompilá el target en `Release` antes de ejecutar `cpack`. CPack empaqueta lo instalado desde el build actual; no recompila automáticamente los cambios nuevos.

El instalador queda en:

```text
build/packages/AcousticSimulator-1.0.0-win64.exe
```

## Paquete portable opcional

Si quieres validar primero que el ejecutable y las DLLs necesarias se empaquetan correctamente, genera un ZIP portable:

```powershell
cpack --config build/CPackConfig.cmake -C Release -G ZIP
```

El ZIP queda en:

```text
build/packages/AcousticSimulator-1.0.0-win64.zip
```

## Qué incluye el paquete

CPack usa las reglas de instalación definidas en `CMakeLists.txt`.

El paquete incluye:

* `bin/AcousticSimulator.exe`
* DLLs desplegadas por vcpkg junto al ejecutable, por ejemplo `glfw3.dll`
* runtime de Visual C++ necesario para ejecutar la aplicación en Windows
* ejecutable configurado como aplicación gráfica de Windows, sin ventana de consola al abrirlo
* acceso directo del menú inicio para `Acoustic Simulator`
* acceso directo del escritorio `Acoustic Simulator.lnk`
* icono embebido en el ejecutable desde `assets/icons/acoustic-simulator.ico`

El paquete no incluye:

* `ambiente3d_tests.exe` ni librerías de GoogleTest
* logs de desarrollo, por ejemplo `core_timing.log`
* archivos temporales de build o paquetes previos

El acceso directo del escritorio se crea explícitamente desde `CPACK_NSIS_EXTRA_INSTALL_COMMANDS` en `CMakeLists.txt` y apunta a:

```text
$INSTDIR/bin/AcousticSimulator.exe
```

Al desinstalar la aplicación, `CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS` elimina ese acceso directo del escritorio.

La esquina de la ventana usa el mismo icono embebido en el ejecutable mediante la integración GLFW/Win32 de `src/gui/App.cpp`.

## Verificación

Después de generar el instalador, verifica que existe el archivo:

```powershell
Test-Path .\build\packages\AcousticSimulator-1.0.0-win64.exe
```

La salida esperada es:

```text
True
```

También puedes revisar el contenido instalado probando primero el ZIP:

```powershell
tar -tf .\build\packages\AcousticSimulator-1.0.0-win64.zip
```

La salida debe incluir, al menos:

```text
AcousticSimulator-1.0.0-win64/bin/AcousticSimulator.exe
AcousticSimulator-1.0.0-win64/bin/glfw3.dll
```

Para verificar el comportamiento del instalador NSIS, instala el `.exe` generado y confirma que se haya creado este acceso directo:

```text
Escritorio/Acoustic Simulator.lnk
```

Luego desinstala la aplicación y confirma que el acceso directo también se haya eliminado.

## Problemas comunes

| Problema | Causa probable | Solución |
|---|---|---|
| `Cannot find NSIS compiler makensis` | NSIS no está instalado o CPack no lo puede encontrar. | Instalá NSIS, abrí una terminal nueva y verificá `makensis /VERSION` o `& "C:\Program Files (x86)\NSIS\makensis.exe" /VERSION`. |
| CMake no encuentra GLFW, GLAD o ImGui | Falta configurar vcpkg o instalar dependencias. | Repetí los pasos de `docs/guides/installation.md`, especialmente la instalación con vcpkg. |
| El paquete no contiene DLLs necesarias | La app no se compiló en `Release` antes de empaquetar. | Ejecutá `cmake --build build --config Release --target AcousticSimulator` antes de `cpack`. |
| El instalador queda fuera de `build/packages` | La configuración de CPack no está actualizada. | Regenerá CMake con `cmake -S . -B build ...`. |
| El instalador no crea el acceso directo del escritorio | El instalador fue generado antes de usar `CPACK_NSIS_EXTRA_INSTALL_COMMANDS`. | Regenerá CMake y luego generá de nuevo el instalador con `cpack --config build/CPackConfig.cmake -C Release -G NSIS64`. |
| El instalador no refleja cambios recientes de la GUI | Se ejecutó CPack sin recompilar el target `Release`. | Volvé a ejecutar `cmake --build build --config Release --target AcousticSimulator` y después `cpack`. |

## Flujo reproducible completo

Para una máquina nueva, el flujo correcto es:

1. Seguir `docs/guides/installation.md` completo.
2. Verificar que NSIS esté instalado.
3. Configurar CMake con el toolchain de vcpkg.
4. Compilar `AcousticSimulator` en `Release`.
5. Ejecutar CPack con `-G NSIS64`.
6. Confirmar que `build/packages/AcousticSimulator-1.0.0-win64.exe` existe.
7. Instalar el `.exe` generado y confirmar que existe `Acoustic Simulator.lnk` en el escritorio.
