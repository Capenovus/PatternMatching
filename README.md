# PatternMatching

### Features
- Match and replace patterns in a file
- Wildcard support in match pattern and replace pattern
- Toggleable overlap matching (disabled by default [slow]: pattern = aaaaa, match aa => 4 results with overlap, 2 without)
- Create config files with multiple patterns

### Usage
```
Arguments

-f file                            | file to match in
-c file                            | load pattern config file
-p matchpattern:replacepattern     | match and replace pattern
-o                                 | (Optional)Match Patterns with overlap

Example Usage:
PatternMatching.exe -f owo.txt -c config.txt
PatternMatching.exe -f owo.txt -p "01 02 03:02 03 04"
```
