
import os
import posixpath
import subprocess
import sys

import action_tree
import cmd_env
import optparse


urls = {
    "third_party": "http://nativeclient.googlecode.com/svn/trunk/nacl/googleclient/third_party@168",
    "nacl": "git://repo.or.cz/nativeclient.git",
    "binutils": "git://repo.or.cz/nacl-binutils.git",
    "gcc": "git://repo.or.cz/gcc/nacl-gcc.git",
    "glibc": "git://repo.or.cz/glibc/nacl-glibc.git",
    "python": "http://www.python.org/ftp/python/2.6.1/Python-2.6.1.tar.bz2",
    }

tag_name = None

# urls["nacl"] = "local/nativeclient"
# urls["binutils"] = "local/binutils"
# urls["gcc"] = "local/gcc"
# urls["glibc"] = "local/glibc"


def clone_or_pull(env, url, dest_path):
    if os.path.exists(dest_path):
        if tag_name is None:
            env.cmd(cmd_env.in_dir(dest_path) + ["git", "pull"])
    else:
        env.cmd(["git", "clone", url, dest_path])
    if tag_name is not None:
        env.cmd(cmd_env.in_dir(dest_path) + ["git", "checkout", tag_name])


def stamp_file(filename):
    fh = open(filename, "w")
    fh.close()


def is_elf_file(filename):
    fh = open(filename)
    header = fh.read(4)
    fh.close()
    return header == "\x7fELF"


class NaClBuild(object):

    def __init__(self, env, dir_path):
        self._env = env
        self._dir_path = dir_path

        def add_path(subdirs):
            path = ":".join(
                [os.path.join(self._dir_path, subdir) for subdir in subdirs] +
                [os.environ["PATH"]])
            return ["env", "PATH=%s" % path]

        subdirs = ["googleclient/native_client/scons-out/dbg-linux/staging",
                   "googleclient/native_client/ncv",
                   "bin"]
        self._path_env = cmd_env.PrefixCmdEnv(add_path(subdirs), env)
        self._gcc_override_env = cmd_env.PrefixCmdEnv(
            add_path(subdirs + ["override-bin"]), env)

    def checkout_thirdparty(self, log):
        self._env.cmd(["svn", "checkout", urls["third_party"],
                       "googleclient/third_party"])

    def checkout_nacl(self, log):
        clone_or_pull(self._env, urls["nacl"],
                      "googleclient/native_client")

    def checkout_binutils(self, log):
        clone_or_pull(self._env, urls["binutils"],
                      "googleclient/native_client/tools/BUILD/binutils-2.18")
        stamp_file("googleclient/native_client/tools/BUILD/stamp-binutils-2.18")

    def checkout_gcc(self, log):
        clone_or_pull(self._env, urls["gcc"],
                      "googleclient/native_client/tools/BUILD/gcc-4.2.2")
        stamp_file("googleclient/native_client/tools/BUILD/stamp-gcc-4.2.2")

    def checkout_glibc(self, log):
        clone_or_pull(self._env, urls["glibc"],
                      "googleclient/native_client/glibc")

    def fetch_sqlite(self, log):
        self._env.cmd(["mkdir", "-p", "debs"])
        url = "http://archive.ubuntu.com/ubuntu/pool/main/s/sqlite3/sqlite3_3.6.10-1.dsc"
        self._env.cmd(cmd_env.in_dir("debs") + ["dget", "-x", url])

    def fetch_python(self, log):
        self._env.cmd(["wget", "-c", urls["python"],
                       "-O", posixpath.basename(urls["python"])])

    def unpack_python(self, log):
        self._env.cmd(["tar", "-xjf", posixpath.basename(urls["python"])])

    @action_tree.action_node
    def checkout(self):
        return [self.checkout_thirdparty,
                self.checkout_nacl,
                self.checkout_binutils, #2
                self.checkout_gcc, #3
                self.checkout_glibc, #
                self.fetch_sqlite,
                self.fetch_python,
                self.unpack_python]

    def build_toolchain(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/tools")
                      + ["make"])

    def build_nacl(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client")
                      + ["./scons"])

    def test_nacl(self, log):
        self._env.cmd(
            cmd_env.in_dir("googleclient/native_client")
            + ["./scons", "run_all_tests"])

    def configure_glibc(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/glibc")
                      + ["./myconfig.sh"])

    def build_glibc(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/glibc/build")
                      + ["make"])

    def test_glibc_static(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/glibc")
                      + ["./make-example.sh"])
        self._path_env.cmd(
            ["ncval_stubout.py", "googleclient/native_client/glibc/hellow"])
        self._path_env.cmd(["sel_ldr", "-d", "googleclient/native_client/glibc/hellow"])

    def _run_dynamic(self, args):
        self._path_env.cmd(
            ["sel_ldr",
             "-d", "googleclient/native_client/install-stubout/lib/ld-linux.so.2",
             "--", "--library-path", "googleclient/native_client/install-stubout/lib"]
            + args)

    def test_glibc_dynamic(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/glibc")
                      + ["./make-example-dynamic.sh"])
        self._run_dynamic(["googleclient/native_client/glibc/hellow-dyn"])

    def install_glibc(self, log):
        self._env.cmd(cmd_env.in_dir("googleclient/native_client/glibc/build")
                      + ["make", "install"])

    def stub_out(self, log):
        self._env.cmd(["rm", "-rf",
                       "googleclient/native_client/install-stubout"])
        self._env.cmd(["cp", "-a",
                       "googleclient/native_client/install",
                       "googleclient/native_client/install-stubout"])
        self._path_env.cmd(
            ["sh", "-c",
             "ncval_stubout.py googleclient/native_client/install-stubout/lib/*.so"])

    def build_sqlite(self, log):
        self._gcc_override_env.cmd(
            cmd_env.in_dir("debs/sqlite3-3.6.10")
            + ["dpkg-buildpackage", "-b", "-us", "-uc", "-rfakeroot"])

    def configure_python(self, log):
        self._gcc_override_env.cmd(cmd_env.in_dir("Python-2.6.1")
                                   + ["i386", "./configure"])

    def build_python(self, log):
        self._gcc_override_env.cmd(
            cmd_env.in_dir("Python-2.6.1") + ["make"])

    def _get_install_dir(self, subdir_name):
        install_dir = os.path.join(self._dir_path, "install-trees", subdir_name)
        self._env.cmd(["rm", "-rf", install_dir])
        self._env.cmd(["mkdir", "-p", install_dir])
        return install_dir

    def _rewrite_for_nacl(self, install_dir):
        for dir_path, dirnames, filenames in os.walk(install_dir):
            for filename in filenames:
                pathname = os.path.join(dir_path, filename)
                if is_elf_file(pathname):
                    self._path_env.cmd(["ncrewrite", pathname])

    def install_python(self, log):
        install_dir = self._get_install_dir("python")
        self._gcc_override_env.cmd(
            cmd_env.in_dir("Python-2.6.1")
            + ["make", "install", "DESTDIR=%s" % install_dir])
        self._rewrite_for_nacl(install_dir)

    def install_sqlite(self, log):
        install_dir = self._get_install_dir("sqlite")
        self._env.cmd(["dpkg-deb", "-x", "debs/libsqlite3-0_3.6.10-1_i386.deb",
                       install_dir])
        self._rewrite_for_nacl(install_dir)

    def build_python_extension(self, log):
        self._env.cmd(
            ["rm", "-rf", "googleclient/native_client/python_extension/build"])
        self._gcc_override_env.cmd(
            cmd_env.in_dir("googleclient/native_client/python_extension") +
            ["python", "setup.py", "install", "--install-lib=../imcplugin"])
        self._path_env.cmd(
            ["ncrewrite", "googleclient/native_client/imcplugin/nacl.so"])

    def test_libraries(self, log):
        # Minimal library test.  Tests that we can load more libraries
        # than just libc.so, and that libraries can call via the PLT.
        gcc = "nacl-glibc-gcc"
        self._path_env.cmd([gcc, "-shared", "-fPIC", "tests/libhello.c",
                            "-o", "tests/libhello.so"])
        self._path_env.cmd([gcc, "tests/hello.c", "tests/libhello.so",
                            "-o", "tests/hello"])
        # Check that the executable runs natively as well as under sel_ldr.
        subprocess.check_call(["./tests/hello"])
        self._path_env.cmd(["ncrewrite", "tests/libhello.so", "tests/hello"])
        self._run_dynamic(["./tests/hello"])

    def test_dlopen(self, log):
        self._path_env.cmd(["nacl-glibc-gcc", "tests/dlopen.c", "-ldl",
                            "-o", "tests/dlopen"])
        self._path_env.cmd(["ncrewrite", "tests/dlopen"])
        self._run_dynamic(["./tests/dlopen"])

    def test_static_dlopen(self, log):
        self._path_env.cmd([
                "nacl-glibc-gcc", "-static",
                "-Wl,-T,googleclient/native_client/glibc/elf_i386.x",
                "tests/dlopen.c", "-ldl", "-o", "tests/dlopen-static"])
        self._path_env.cmd(["ncrewrite", "tests/dlopen-static"])
        self._path_env.cmd(["ncval_stubout.py", "tests/dlopen-static"])
        self._path_env.cmd(["sel_ldr", "-d", "-E", "LD_LIBRARY_PATH=googleclient/native_client/install-stubout/lib", "./tests/dlopen-static"])

    def test_gcc_wrapper(self, log):
        self._path_env.cmd(["python", "gccwrapper_test.py"])

    def test_python(self, log):
        self._path_env.cmd(["ncrewrite", "Python-2.6.1/python"])
        self._run_dynamic(["Python-2.6.1/python", "-c", 'print "Hello world!"'])

    @action_tree.action_node
    def build(self):
        return [self.build_toolchain,
                self.build_nacl,
                self.configure_glibc,
                self.build_glibc,
                self.install_glibc,
                self.stub_out,
                self.build_sqlite,
                self.configure_python,
                self.build_python,
                self.install_python,
                self.install_sqlite,
                self.build_python_extension]

    @action_tree.action_node
    def test(self):
        return [self.test_gcc_wrapper,
                self.test_nacl,
                self.test_glibc_static,
                self.test_glibc_dynamic,
                self.test_libraries,
                self.test_dlopen,
                self.test_static_dlopen,
                self.test_python]

    @action_tree.action_node
    def all(self):
        return [self.checkout, self.build, self.test]


class DryRunEnv(object):

    def cmd(self, args, **kwargs):
        print " ".join(args)


def main(args):
    parser = optparse.OptionParser()
    parser.add_option("--dry-run", action="store_true", dest="dry_run",
                      help="Print commands instead of running them")
    options, args = parser.parse_args(args)
    if options.dry_run:
        env = DryRunEnv()
    else:
        env = cmd_env.BasicEnv()
    tree = NaClBuild(env, os.getcwd())
    action_tree.action_main(tree.all, args)


if __name__ == "__main__":
    main(sys.argv[1:])
