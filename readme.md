
# base64 ( ver 1.0 )
base64 encode and decode tool for handle file or string to file or console

## Features
encode
decode

## Do not support yet
+ operator overload

## Usage
+ module and const integral
```c++
base64.exe Encode and Decode tool (20220915)

    usage: base64.exe <options>
    -encode                   : encode input
    -decode                   : decode input
    -input <string>
    -inputfile <filename>
    -outputfile <filename>
    -needreturn               : when encode one line 76 chars
```
                
## Sample
encode file
```c++
base64.exe -encode -inputfile a -outputfile b -needreturn
```

decode file
```c++
base64.exe -decode -inputfile b -outputfile a
```

encode string through command line
```c++
base64.exe -encode -input sometext -outputfile b
```

decode string through command line
```c++
base64.exe -decode -input somebase64string -outputfile a
```

output to console without -outputfile option
```c++
base64.exe -decode -input somebase64string
base64.exe -encode -inputfile a
```

## License
+ Licensed under the [MIT License](https://www.lua.org/license.html).

