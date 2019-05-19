# memusage
Any HPC workflow invariably starts by writing a job submission scrip that codes
the workflow and, most importantly, declares the amount of resources required.
These resources vary in all HPC systems but they always include the number of
cores to be used, the amount of time needed and the amount of memory required.
Of all these, the amount of memory is usually the most difficult to estimate.
And even when an estimation is possible describing is usage throughout the
duration of the job, which may have impact in any future debugging, is usually
much harder. In this situations it would desirable if these HPC systems could
give the users an automatic tool to monitor memory usage throughout the
duration of the job.
This work is the solution developed by us, as a very simple forking code,
for this situation.
