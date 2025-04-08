# Build Your Own wc Tool

TODO: Create class Design

This challenge has been taken from Coding challenges FYI site present 
[here](https://codingchallenges.fyi/challenges/challenge-wc).

This document is focused on first understanding the problem and then using it as a way to define our objectives which
would then be explored in architecture document which would talk about the different things that are necessary to 
achieve our solution in code.

# What is `wc`?

The `wc` is a tool from Linux that is used to count Words, line, characters and bytes. 
The example [text](https://www.dropbox.com/scl/fi/d4zs6aoq6hr3oew2b6a9v/test.txt?rlkey=20c9d257pxd5emjjzd1gcbn03&dl=1)
must be downloaded for the project and that is what we will use for designing our approach.

# The option `-c`, `-l`, `-w` and `-m`

This option `-c` is used to find the total number of bytes present in the file. The output shoudl be as following:

```
$ wc -c test.txt
  342190 test.txt
```

The option `-l` counts the total number of lines present in the file and should have the following output:

```
$ wc -l test.txt
    7145 test.txt
```

The option `-w` talks about total number of words that are present in the file and would have the following output:

```
$ wc -w test.txt
   58164 test.txt
```

The option `-m` refers to the locale we are using and is used to find the multibyte character count in file:

```
$ wc -m test.txt
  342190 test.txt
```

# When providing no options?

The following is the output when not providing any options:

```
$ wc test.txt
    7145   58164  342190 test.txt
```

# When no file name is provided

When we see that there are no filenames provided - it takes the input as `stdin` and then we never provide the name of
the file as output

```
$ cat test.txt | wc
    7145   58164  342190
```

> [!IMPORTANT]
> If I were to look at the spaces shown for each output we can see that there is already a GAP of white space on their 
> left which indicates `wc` tool somehow knows the length of each of the different output options and is formatting the
> spaces of each of the output as well
>
> This leads to the following realizations:
> 
>   FIRST - By default we have to do three calculations together - bytes, lines and words, in main algorithm to count,
> this is confirmed when providing no options
> 
>   SECOND - There is an output formatter that will be used to determine the number of whitespaces required to make
> our content right sensitive. The pattern is as following:
> 
> ```<space><max_length+1 for numbers>```
>
>   THIRD - In case of stdin we never provide the file name in our output.
> 

# How are parameters provided together?

Here are the various different outputs of my trial when trying to determine how are the parameters going to parsed:

```
$ wc -l -m test.txt
    7145  342190 test.txt

$ wc -lm test.txt
    7145  342190 test.txt

$ wc -lwc test.txt
    7145   58164  342190 test.txt

$ wc -wlc test.txt
    7145   58164  342190 test.txt

$ wc -wlcm test.txt
    7145   58164  342190  342190 test.txt

$ wc -wc test.txt test2.txt
   58164  342190 test.txt
   58164  342190 test2.txt
  116328  684380 total

$ cat test.txt | wc test2.txt
    7145   58164  342190 test2.txt

$ wc test.txt -lwc
    7145   58164  342190 test.txt
```

> [!IMPORTANT]
> This means that output has to be parameterised properly and provided with a hypen `-` to be parsed correctly as it 
> would provide what operations has to be performed by default our output would only contain lines, words and bytes in
> that specific order and after that multibyte based on the local.
>
> The parameters are position independent on my system.
>
> Our output would depend on the flags that have been activated by the input parsed but also looking at the various
> different files provided we would have to adjust our output formatting on the total as provided from the list as well.
>
> We also ensure that we will ignore stdin when we have some other file written in our command line.
