#!/usr/bin/env python

top = '.'
out = 'build'


subdirs = ['cpp', 'chat-example']


def options(opt):
	opt.load('waf_unit_test')
	for subdir in subdirs:
		opt.recurse(subdir)


def configure(conf):
	conf.load('waf_unit_test')
	for subdir in subdirs:
		conf.recurse(subdir)


def build(bld):
	for subdir in subdirs:
		bld.recurse(subdir)

	# add post-build function to show the unit test result summary
	from waflib.Tools import waf_unit_test
	bld.add_post_fun(waf_unit_test.summary)

