# Tests de desarrollo

Los tests viven en `src/test` y se ejecutan en un build separado para no ensuciar el build principal del programa.

## Configurar

Desde la raiz del proyecto:

```powershell
cmake -S . -B build-tests -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DACOUSTIC_BUILD_TESTS=ON
```

## Compilar

```powershell
cmake --build build-tests --config Debug --target ambiente3d_tests
```

## Ejecutar

```powershell
ctest --test-dir build-tests -C Debug --output-on-failure
```

## Nota

El build normal sigue usando `build` y no incluye tests por defecto. Los tests solo se agregan cuando `ACOUSTIC_BUILD_TESTS=ON`.
