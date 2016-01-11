# expandconfig
Small program which reads text files where there can be alternative text parts and return text files with all possible combinations.

## Backstory
I have to make numerical experiments using a program which reads config files. Like in all numerical experiments, some parameters in the config file have to varied. Instead of manually creating hundreds of config files I decided to write a small program which does this for me.

Fortunately, this program works not just on config files but on all text files (at least I guess so).

## Example
Consider the config file `config.cfg`:

    this = [[ 1   || 2    ]]
    that = [[ 0.5 || 0.75 ]]

A call `expandconfig config.cfg` will return four different files `config.cfg.0000001.exp` upto `config.cfg.0000004.exp` with

    this =  1   
    that =  0.5 

and

    this =  2    
    that =  0.5 

and

    this =  1   
    that =  0.75  

and finally

    this =  2    
    that =  0.75 

## "Language"

Text parts with alternatives are enclosed by `[[` and `]]`, where multliple alternatives are separeted by `||`. In the text parts which are common in all desired output files, you can't have any `[[`, and in the alternatives you can't have any `]]` or `||`. Every config file which is as whole matched by the regular expression

    /[^\[\[]*(\[\[([^(\]\])])*(||[^\]\]]*)*\]\])*[^\[\[]*/g

should work fine. You can test it [here](http://regexr.com/3ci3r).

## Bugs?
Everything works as intended at the moment.

## Future improvements?
 - Giving "tags" to text parts with alternatives, such that parts with the same "tag" are alternated together instead of independend of each other.
 - Improving the quality of the code.
 - Adding (better) error handling.
 - Additionaly features.

