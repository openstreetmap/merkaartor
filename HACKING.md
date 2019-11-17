# Hacking Merkaartor

Here are some tips if you want to hack or debug Merkaartor.

## Tag templates

Tag templates are XML files under the ''Templates'' directory. See ''default.mat'' as a
reference. Basically a `widget` tag specifies a choice in the ''Properties'' dialog,
and `value` tags choices for the value. It's not pretty or easy to use.

Before you start hacking, drop an email to the list. I think it's worth adopting
JOSM presets instead, as we gain a pretty exhaustive database and people will have
an easier time switching between Merkaartor and JOSM, which is always good!

## Translations

There is more information in the translations/README.md. However, if you just want to
contribute to translations, either by translating an existing language or adding a
new one, visit our Transifex page and start there. There are a few words on our
web, http://merkaartor.be/p/docs/localization, together with a link to the
Transifex page.

## Rendering styles

Rendering styles are stored in the ''Styles/'' directory, but they are not to be edited by hand. Use Merkaartor's
style editor instead, and let it save for you.

If you want to add a new style, do so freely. Keep changes to existing files to
a minimum, so the overall look, feel, and rendering speed won't change much.

I would be very happy if someone made a comprehensive style suited for editing
POIs, yet easy to navigate.

## Sanitizers

Sanitizers can be enabled by the SANITIZE option:

```
qmake -r SANITIZE=1
qmake -r SANITIZE=2
qmake -r SANITIZE=custom_sanitizer
```

The option =1 enables the address and undefined sanitizers. The option =2
enables the thread sanitizer. Enything else will be just passed to the compiler, so
you can specify your own. It might be useful to run a compile with Clang instead
of GCC for different options/implementations.

It might complain about some stuff in other libraries (gdal, Qt), so keep in
mind these are not our responsibilities for the most case. Usually all the bugs
detected are severe and should be reported. Even running Merkaartor with
a sanitizer enabled and reporting bugs is a huge benefit, though there is a severe
performance hit involved.

## Compiling with Clang

You can compile Merkaartor with Clang, using standard Qt approach:

```
qmake -spec linux-clang
```

## Variable naming convention

You might have noticed the strange naming convention for variables. They are
often prefixed with a definite or indefinite article, like aLayer and theLayer.
It's been in Merkaartor since the very beginning of the Git history.

The exact meaning in Merkaartor is still a bit cloudy, and I don't expect
anyone to keep this notation in new code, but it might be a good idea in some
cases. However, I will not accept commits that try to change this notation just
for the sake of changing.

A possible representation was given by @PeterMortensen (see [issue #184 on github](https://github.com/openstreetmap/merkaartor/issues/184) for a bit more details)

One example was in the commercial C++ framework Think Class Library (TCL),
bundled with the THINK C compiler. TCL was similar to Qt, for creating
GUI-based applications, etc. This was in the early 1990s for MacOS (before both
Mac OS X and PowerPC).

It used the definite or indefinite article prefix, the indefinite for parameter
names of a function and the definite for scalar member variables (not class
instances). It also used the "its" prefix for members that were class instances
(this was before references in C++, so all access to class instances were
through pointers). If I remember correctly, this was to indicate ownership
(responsibility for destruction, etc.).

The TCL framework also had the convention of "I" for second-level
initialisation of objects after creation (possibly due to limitations in the
compiler support of C++ features - I don't remember).


