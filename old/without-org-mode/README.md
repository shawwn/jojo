### About do not use org-mode

This `src/` tree contains an effort to quit using org-mode,
and to separate one `.org` file to many `.hpp` and `.cpp` files.

The experience are :
- The overall build time went from 10s to 130s.
- It become header to change the code :
  - changing naming in one file,
    becomes changing naming in many files and changing file name.
  - re-structuring the code in org-mode headline tree,
    because re-structuring the source-tree in file system.

Thus, I decide to not quit org-mode,
to make the code easier to change,
easier to re-structuring
and easier to review.
