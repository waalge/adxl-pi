# adxl-pi

Aims:

* collect accelerometer data using a pi zero w and adxl345
* learn to develop quickily and efficiently on remote + lower power devices. 

## Setup 

### Setup remote

On remote device (kiwipi) 
```
git init --bare ~/repo/adxl-pi.git
```

On local 
```
git remote add kiwipi ssh://kiwipi/~/repo/adxl-pi.git
```

Add post-receive hook on remote that links a tracked file. 
In `~/repo/adxl-pi.git/hooks/post-receive` write 

```bash
#!/bin/bash
GIT_WORK_TREE=~/repo/adxl-pi.git
export GIT_WORK_TREE
read oldrev newrev ref
git show build:post-receive.sh | bash -s -- $ref
```
Set file to executable (`chmod +x post-receive`). 

TODO : what does this first bit do?

Git pipes to bash the version of `post-receive.sh` on branch `build` with a single argument, the ref. 

### gitignore 

Start from [github's](https://github.com/github/gitignore/blob/master/C.gitignore). 

Add: 

* build dir `build/`
* ccls cache from editor `.ccls-cache` 

### post-receive shell + make

This checks whether the push was to the branch `build`.
If yes, then copy the contents of the repo to a working directory, and run `make`.

It remains to write the Makefile.
