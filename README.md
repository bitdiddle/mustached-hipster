mustached-hipster
=================

###	Intro
*	experiment with cache replacement policy, inclusive/exclusive multi-level cache and evaluation with Intel Pin

*	team
	*	alex (team leader)
	*	taylor (guru)
	*	larry (programmer)

*	course work for Computer Architecture 14'
	*	[CA:PinBranch](http://202.120.38.22:1000/wiki/index.php/CA:PinBranch)

*	develop environment
	*	pin-2.13-65163-clang.5.0-mac

###	Charts
![FIFO](results-FIFO.png)
![LRU](results-LRU.png)
![RANDOM](results-RANDOM.png)

### Ref
*	[Pin: Pin 2.13 User Guide](https://software.intel.com/sites/landingpage/pintool/docs/65163/Pin/html/)
	*	[Pin: Instrumentation arguments](https://software.intel.com/sites/landingpage/pintool/docs/61206/Pin/html/group__INST__ARGS.html)


### Journal
*	bsdtar & gnutar: tar zcf data1.tgz data1
*	<pre>
| -            | gnutar   | bsdtar     |
| Writes       | 294242   | 431184044  |
| Write Misses | 18557    | 72480524   |
| Reads        | 522704   | 893101137  |
| Read Misses  | 87312    | 132932402  |
| Misses       | 105869   | 205412926  |
| Accesses     | 816946   | 1324285181 |
| Miss Ratio   | 12.9591% | 15.5112%   |
</pre>
*	and time
*	<pre>
	real	0m31.516s
	user	0m30.834s
	sys	0m0.508s
</pre>
