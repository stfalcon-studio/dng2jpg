Конвертер графических файлов из формата DNG в формат JPEG.
=======

Требования при сборке:
-----------

Для сборки необходим CMake и mingw-4.8, и настроенная среда с наличием утилиты make (например, MSYS или Qt SDK).


Требования при запуске:
-----------

Должен быть установлен кодек от Adobe: DNGCodec.dll http://www.adobe.com/support/downloads/product.jsp?product=194&platform=Windows

Использование
-----------

```
dng2jpg <input_file> <output_file>
```

Где:
<input_file> - полный путь к входящему файлу DNG
<output_file> - полный путь, куда надо сохранить результат в JPG
