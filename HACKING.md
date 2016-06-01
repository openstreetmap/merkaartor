# Hacking Merkaartor

Here are some tips if you want to hack or debug Merkaartor.

## Tag templates

Tag templates are xml files under the Templates directory. See default.mat as a
refernce. Basically a `widget` tag specifies a choice in the Properties dialog,
and `value` tags choices for the value. It's not pretty or easy to use.

Before you start hacking, drop an email to the list. I think it's worth adopting
JOSM presets instead, as we gain pretty exhaustive database and people will have
easier time switching between Merkaartor/JOSM, which is always good!

## Translations

There is more info in the translations/README.md. However, if you just want to
contribute to translations either by translating existing language or adding a
new one, visit our transifex page and start there. There are a few words on our
web http://merkaartor.be/p/docs/localization , together with link to the
transifex page.

## Rendering styles

Stored in Styles/ directory, but not to be edited by hand. Use Merkaartor's
style editor instead, and let it save for you.

If you want to add a new style, do so freely. Keep changes to existing files to
a minimum, so the overall look, fell and rendering speed won't change much.

I would be very happy if someone made a comprehensive style suited for editing
POIs, yet easy to navigate.

## Address sanitizer

Can be enabled by:

```
qmake -r SANITIZE=1
```

It might complain about some stuff in other libraries (gdal, qt), so keep in
mind these are not our responsibilities for the most case. Usually all the bugs
detected are severe and should be reported. Even running Merkaartor with
sanitizer enabled and reporting bugs is a huge benefit, though there is severe
performance hit involved.

## Variable naming convention

You might have noticed the strange naming convention for variables. They are
often prefixed with a definite or indefinite article, like aLayer and theLayer.
It's been in Merkaartor since the very beginning of git history. I know nothing
about it, so if you have seen it elsewhere, let me know!

My best bet, based on some observation and common sense is that theVariable
would represent the same object during it's lifetime. aVariable could change
objects, for example if it's used in a loop, iterating over layers.

I don't expect anyone to keep this notation in new code, but it might be a good
idea in some cases. However, I will not accept commits that try to change this
notation just for the sake of changing.
