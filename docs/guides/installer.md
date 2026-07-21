# Generar el instalador de Windows

Pasos mínimos para generar el instalador `.exe` de `AcousticSimulator`.

## Requisito

Tené el entorno del proyecto configurado según `docs/guides/installation.md` y NSIS instalado.

Podés verificar NSIS con:

```powershell
makensis /VERSION
```

## Pasos

Ejecutá estos comandos desde la raíz del proyecto:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DACOUSTIC_BUILD_TESTS=OFF
cmake --build build --config Release --target AcousticSimulator
cpack --config build/CPackConfig.cmake -C Release -G NSIS64
```

## Resultado

El instalador queda en:

```text
build/packages/AcousticSimulator-1.0.0-win64.exe
```
