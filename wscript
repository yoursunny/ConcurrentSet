# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.1'
APPNAME='ConcurrentSet'

from waflib import Build, Utils, Logs

def options(opt):
    opt.load(['compiler_c', 'gnu_dirs'])

    opt.add_option('--with-tests', action='store_true', dest='with_tests', help='''build unit tests''')
    opt.add_option('--debug', action='store_true', dest='debug', help='''debugging mode''')

def configure(conf):
    conf.load(['compiler_c', 'gnu_dirs'])

    conf.env.append_value('CFLAGS', ['-std=gnu99'])

    if conf.options.debug:
        conf.define('_DEBUG', 1)
        conf.env.append_value('CFLAGS', ['-O0', '-Wall', '-Werror', '-g3'])
    else:
        conf.env.append_value('CFLAGS', ['-O3', '-g'])

    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = True
        conf.check_cc(lib='cunit', include='CUnit/CUnit.h', uselib_store='CUNIT', mandatory=True)

def build (bld):
    bld(target='concurrent-set',
        features='c cstlib',
        source=bld.path.ant_glob(['src/*.c']),
        includes='src',
        install_path='${LIBDIR}')

    bld.install_files('${INCLUDEDIR}', 'src/concurrent-set.h', cwd='src')

    if bld.env['WITH_TESTS']:
        bld(target='unit-tests',
            features='c cprogram',
            source=bld.path.ant_glob(['tests/*.c']),
            use='concurrent-set CUNIT',
            includes='src',
            install_path=None)
