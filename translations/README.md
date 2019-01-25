Merkaartor currently uses Qt Linguist and Transifex to translate stuff. Although
you might be tempted to change translations in git repo directly, please don't.
Translation files are meant to be updated via Transifex.com only, and pulled
into the repository before version release.

The regular workflow is:

0) From time to time, update translation files on Transifex:
    
    $ lupdate -no-obsolete Merkaartor.pro
    $ tx push -s

   This pushes the original english strings to Transifex, updating the database and
   allowing new strings to be downloaded.

   If transifex complains about numerous forms, you need to open linguist,
   search for all strings with %n and fill in the appropriate english forms.
   Then repeat the push.

1) Before release, Transifex data should be pulled and committed into the
repository. This is not necessary, but serves as a backup in case something
terrible happens to Transifex.

    $ tx pull
    $ git commit translations/*.ts

2) During release, the regular qt tools are used to bundle translations. Care
should be taken to include new translations in Merkaartor.pro, so the .qm files
get generated properly. A language list should be also updated in [1] to
reflect newly added language and translation status (poorly translated
languages are marked).


[1] src/Preferences/PreferencesDialog.cpp
