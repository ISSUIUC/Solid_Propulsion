# Solid Propulsion

> "Don't Let Team Center stop you, cause' we can't stoppin."
> - The forebears

This repo serves as a monolithic repository (a "monorepo") for any and all contributions done for the Solid Propulsion team at the Illinois Space Society
It serves both to track CAD contributions as snapshots (wherein CAD from older commits can be examined to see older versions) and track code as any normal Git repository.
At this time, there are no plans to separate CAD and code, although the idea should be strongly considered due to the incompatible semantics of versioning that CAD and code have.

# Contributing

We have a few rules for adding new items to this repository that depend on what manner of contribution you plan to add.

A few general rules of thumb:
- Do not commit directly to main, open a branch and create a pull request.
- Do not force merge your pull requests. If the contribution is CAD, it will be automatically by our CI/CD pipeline. If it is code, then your commit should be tested before it gets merged.
- When creating branches, adhere to the branch structure which is included below.

| Type of Commit   | Branch Prefix  |
|--------------- | --------------- |
| CAD   | `cad/`   |
| New code feature   | `feat/`   |
| Code refactor   | `<refactor/`   |
| Bug fix   | `fix/`   |
| Repository chore | `chore/` |

Note: While we do not presently have more than 1 codebase, in the event we have multiple (which will be an eventuality given we will be storing our in-house Python data analysis software here as well as our firmware), one should prefix all normal code branches with the name of the codebase that you are working on.
EG: If we had a new feature for our firmware, the branch prefix `firmware/feat/` would be used instead of `feat/` as the feature now exists in the context of one of our codebases.

## For CAD

It would be preferable if none of the files you are adding have spaces in their name, as well as folders.
Name your branch based on the overall changes you've made. 
For example, if you were to have created a new part and modified an existing one to create a new crossbeam, name the branch `cad/add-new-crossbeam`.
Do not name it after any individual changes you've made, rather the sum total.

## For code
It is mandatory none of the files you are adding have spaces in their name, as well as folders.
Name your branch based on the feature you've added, the bug you've fixed, or the item(s) you've refactored.

### Commits
We follow [conventional commits](https://www.conventionalcommits.org/en/v1.0.0/) when doing any squash merge commits when merging a branch.
When doing normal commits, ensure your commits are reasonably well detailed as to what they change.

#### When starting a new codebase
Starting a new codebase usually means transitioning from a nonworking prototype to a minimum viable product very rapidly.
We discourage committing anything until the codebase is in a working state, as zealously logging changes in Git when the code is in such a state of flux hinders productivity.

