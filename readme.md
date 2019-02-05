# Topion: "DRY" C++ command line option parser (Un*x-style)

Hiroyuki Hanada https://www.hiroyuki-hanada.info/ hana-hiro@live.jp

## Outline

To accept such a command line options as
```sh
$ ./myprogram --mode=read --trials=10000 MyData1.csv MyData2.txt
# Let "--mode" be mandatory, while "--trials" be optional (by default 1000)
```
the following program in C++ with Topion will do above:
```cpp
#include "topion.hpp"

struct MyOptions : public Topion{
    TOPION_ADD_MA(std::string, mode, "MODE", "Mode of the program");
    // MA: Mandatory + Argument name ("MODE")
    TOPION_ADD_ODA(std::size_t, trials, 1000, "UINT", "Number of trials");
    // ODA: Optional + Default value (1000) + Argument name ("UINT")
    TOPION_STANDALONE_ATLEAST(1, "[FILES...]");
};

#include <iostream>

int main(int argc, char ** argv){
    MyOptions mo;
    try{
        mo.topion_parse(argc, argv);
    }catch(const topion_parsing_error & e){
        std::cerr << e.what() << std::endl;
        mo.topion_usage(std::cerr);
        return -1;
    }
    std::cout << "Mode: " << mo.mode << std::endl;
    std::cout << "Trials: " << mo.trials << std::endl;
    for(size_t i = 0; i < mo.topion_standalone_size(); ++i){
        std::cout << "File[" << (i+1) << "/" << mo.topion_standalone_size() << "]: " << mo[i] << std::endl;
    }
    return 0;
}
```

Then the program will accept the examples below.
```
$ ./myprogram  --mode=read --trials=10000 MyData1.csv MyData2.txt

Mode: read
Trials: 10000
File[1/2]: MyData1.csv
File[2/2]: MyData2.txt
```

```
# Use default "--trials" value
$ ./myprogram --mode=read MyData1.csv MyData2.txt

Mode: read
Trials: 1000
File[1/2]: MyData1.csv
File[2/2]: MyData2.txt
```

In addition, the program will reject the following commands.
```
$ ./myprogram --mode=read --trials=10000

An error found when parsing the command line: At least 1 standalone parameter is required (specified 0).
```

```
$ ./myprogram --mode=read --trials=-10000 MyData1.csv MyData2.txt

An error found when parsing the command line: Switch "--trials": Value "-10000" is invalid for this switch.
```

Topion is made DRY ([don't repeat yourself](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself)) as possible. Most of existing command line parsers require either or both of the following codes that Topion avoids:

-   The assignment to the result variable separate from the parsing (e.g., `trials = parse_result("trials");`),
-   Specification of data types when finally retrieving the parsed results (e.g., `parse_result<std::size_t>("trials");`).

## Detailed usage

### Define the struct for the parsed result

Do the followings:

1.  Define a `struct` inheriting `public Topion`.
2.  Write any number of `TOPION_ADD_****(...);` needed for your program.

Available `TOPION_ADD_****` are formally defined with the regular expression as `/TOPION_TOPION_ADD_[MO]D?S?A?C?/`, and the arguments are

1.  `T` (any `TOPION_ADD_****` requires): The type of the variable.
    -   In case `T` is `bool`, then the value becomes `true` if the switch is specified and `false` otherwise. In this case, the switch cannot take a parameter string.
    -   Otherwise, the switch must take a parameter string and converts it into the type. See also the `CONV` argument.
    -   In case it is not any of `bool`, `char`, `std::string`, `short`, `unsigned short`, `int` (including the ones modified by `unsigned`, `long` and/or `long long`), `float` or `double`, the option `CONV` is also required.
2.  `var` (any `TOPION_ADD_****` requires): The name of the switch, and also the name of the variable. The name should NOT begin with **topion_** (lowercase, uppercase or mixed) since such a name may be used for Topion-specific behaviors. See also the `short` argument.
3.  `defaultval` (only when `D` is in `****`): The default value of `var`. By default, `var` is initialized by the constructor without any argument.
4.  `short` (only when `S` is in `****`): The short switch name.
    -   In case `var` consists of one character, then only the short option like "`-a`" is available. In this case `short` option cannot be specified.
    -   In case `var` consists of two or more characters and `short` is specified, then the former becomes the long option like "`--abc`" while the latter becomes the short option like "`-a`".
    -   In case `var` consists of two or more characters and `short` is NOT specified, then only the long option like "`--abc`" is available.
5.  `argname` (only when `A` is in `****`): The description of the parameter when displaying the usage. By default `T` is used.
6.  `CONV` (only when `C` is in `****`): How to convert the string to a value of type `T`. For the types above, the conversion function is defined in the library. This must be a functional object of type `bool(void * target, const std::string & s)`, where
    -   `target` is a pointer to `T` at which we store the value (i.e., you call `static_cast<std::string *>(target)) = value`),
    -   `s` is the parameter, and
    -   the returned `bool` value is `true` if the conversion is succeeded or `false` otherwise.
7.  `desc` (any `TOPION_ADD_****` requires): The description of the switch used to displaying the usage.

### Methods

The following methods are implemented in the `Topion` struct.

-   `topion_usage(IO & out, size_t display_width = 70, size_t tab_width = 20)`: Displays the usage to `out` by the operator `<<` (`out` is assumed to be an instance of `std::ostream`, for example, `std::cout` or `std::cerr`.
-   `topion_parse(int argc, char ** argv)`: Parses the command line arguments to store values to the struct.
-   `const std::deque<std::string> & topion_standalones()`: The list of standalone (associated with no switch) parameters.
-   `typename std::deque<std::string>::size_type topion_standalone_size()`: Number of standalone parameters.
-   `const std::string & operator[](typename std::deque<std::string>::size_type p)`: The `p`-th standalone parameter.
-   `void topion_release_parser()`: Releases the memory for information that is used only parsing the options. After this is called, `topion_usage` and `topion_parse` will not work.

### Exceptions

Two exceptions are defined in the library.

-   `topion_definition_error`: Raised when there is an error in defining a switch, e.g., when defining two switches having the same name. A subclass of `std::invalid_argument`.
-   `topion_parsing_error`: Raised when the command line does not meet the requirement of the defined switches, e.g., a negative number is specified for `unsigned int` switch. A subclass of `std::runtime_error`.
