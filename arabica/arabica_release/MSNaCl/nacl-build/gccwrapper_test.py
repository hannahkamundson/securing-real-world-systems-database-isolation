
import os
import shutil
import subprocess
import tempfile
import unittest


# From http://lackingrhoticity.blogspot.com/2008/11/tempdirtestcase-python-unittest-helper.html
class TempDirTestCase(unittest.TestCase):

    def setUp(self):
        self._on_teardown = []

    def make_temp_dir(self):
        temp_dir = tempfile.mkdtemp(prefix="tmp-%s-" % self.__class__.__name__)
        def tear_down():
            shutil.rmtree(temp_dir)
        self._on_teardown.append(tear_down)
        return temp_dir

    def tearDown(self):
        for func in reversed(self._on_teardown):
            func()


def write_file(filename, data):
    fh = open(filename, "w")
    try:
        fh.write(data)
    finally:
        fh.close()


example_prog = """
#include <stdio.h>
int main() {
  printf("%s", ""); /* Tests making a function call */
  return 0;
}
"""


class GccWrapperTest(TempDirTestCase):

    def test_default_output_file_for_linking(self):
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), example_prog)
        subprocess.check_call(["nacl-glibc-gcc", "-Wall", "foo.c"],
                              cwd=temp_dir)
        self.assertEquals(sorted(os.listdir(temp_dir)), ["a.out", "foo.c"])
        # Nops should have been written.  Executable should be runnable.
        subprocess.check_call([os.path.join(temp_dir, "a.out")])

    def test_default_output_file_for_c(self):
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), example_prog)
        subprocess.check_call(["nacl-glibc-gcc", "-Wall", "-c", "foo.c"],
                              cwd=temp_dir)
        subprocess.check_call(["gcc", "foo.o", "-o", "foo"], cwd=temp_dir)
        # Nops should have been written.  Executable should be runnable.
        subprocess.check_call([os.path.join(temp_dir, "foo")])
 
    def test_explicit_output_file(self):
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), example_prog)
        subprocess.check_call(["nacl-glibc-gcc", "-Wall", "foo.c", "-o", "foo"],
                              cwd=temp_dir)
        # Nops should have been written.  Executable should be runnable.
        subprocess.check_call([os.path.join(temp_dir, "foo")])

    def test_building_against_installed_libraries(self):
        # Assumes libreadline5-dev is installed.
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), """
#include <readline/readline.h>
int main()
{
  return 0;
}
""")
        subprocess.check_call(
            ["nacl-glibc-gcc", "-Wall", "foo.c", "-lreadline", "-o", "foo"],
            cwd=temp_dir)
        subprocess.check_call([os.path.join(temp_dir, "foo")])


class GlibcTest(TempDirTestCase):

    def test_linking_with_libpthread(self):
        # Just tests that libpthread initialises OK.
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), example_prog)
        subprocess.check_call(
            ["nacl-glibc-gcc", "-Wall", "foo.c", "-lpthread", "-o", "foo"],
            cwd=temp_dir)
        subprocess.check_call([os.path.join(temp_dir, "foo")])

    def test_linking_with_dubious_options(self):
        # Python links _sqlite.so with -L/usr/lib which causes the
        # linker to drag in the static /usr/lib/libc.a.
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "foo.c"), example_prog)
        subprocess.check_call(
            ["nacl-glibc-gcc", "-Wall", "foo.c", "-lpthread",
             "-L/usr/lib", "-o", "foo"],
            cwd=temp_dir)
        subprocess.check_call([os.path.join(temp_dir, "foo")])
        # TODO: Validate resulting code.  Otherwise broken output is
        # noisy but doesn't signal failure.

    def _check_fstat(self, suffix):
        temp_dir = self.make_temp_dir()
        write_file(os.path.join(temp_dir, "test.c"), r"""
#define _GNU_SOURCE
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
int main()
{
  struct stat%(suffix)s st;
  int fd = open("test", O_RDONLY);
  if(fd < 0) {
    perror("open");
    return 1;
  }
  memset(&st, 0xee, sizeof(st));
  if(fstat%(suffix)s(fd, &st) < 0) {
    perror("fstat");
    return 1;
  }

#define FIELD(name) printf("('%%s', %%li),\n", #name, (long) st.name)
  FIELD(st_dev);
  FIELD(st_ino);
  FIELD(st_mode);
  FIELD(st_nlink);
  FIELD(st_uid);
  FIELD(st_gid);
  FIELD(st_rdev);
  FIELD(st_size);
  FIELD(st_blksize);
  FIELD(st_blocks);
  FIELD(st_atime);
  FIELD(st_mtime);
  FIELD(st_ctime);
  return 0;
}
""" % {"suffix": suffix})
        subprocess.check_call(["nacl-glibc-gcc", "test.c", "-o", "test"], cwd=temp_dir)
        subprocess.check_call(["ncrewrite", "test"], cwd=temp_dir)
        args = ["sel_ldr",
                "-d", os.path.join(os.getcwd(), "googleclient/native_client/install-stubout/lib/ld-linux.so.2"),
                "--", "--library-path", os.path.join(os.getcwd(), "googleclient/native_client/install-stubout/lib"),
                "./test"]
        proc = subprocess.Popen(args, stdout=subprocess.PIPE,
                                stderr=open(os.devnull, "w"),
                                cwd=temp_dir)
        stdout = proc.communicate()[0]
        self.assertEquals(proc.wait(), 0)
        fields = dict(eval("[%s]" % stdout, {}))
        real_stat = os.stat(os.path.join(temp_dir, "test"))
        # Some of these values are dummy values filled out by NaCl.
        self.assertEquals(fields["st_dev"], 0)
        # As a hack, a new st_ino is returned each time by NaCl but we
        # expect it to be small but non-zero.
        assert 0 < fields["st_ino"] < 100, fields["st_ino"]
        # NaCl clears the "group" and "other" permission bits.
        self.assertEquals("%o" % fields["st_mode"],
                          "%o" % (real_stat.st_mode & ~0077))
        self.assertEquals(fields["st_nlink"], real_stat.st_nlink)
        self.assertEquals(fields["st_uid"], -1)
        self.assertEquals(fields["st_gid"], -1)
        self.assertEquals(fields["st_rdev"], 0)
        self.assertEquals(fields["st_size"], real_stat.st_size)
        self.assertEquals(fields["st_blksize"], 0)
        self.assertEquals(fields["st_blocks"], 0)
        self.assertEquals(fields["st_atime"], real_stat.st_atime)
        self.assertEquals(fields["st_mtime"], real_stat.st_mtime)
        self.assertEquals(fields["st_ctime"], real_stat.st_ctime)

    def test_fstat(self):
        self._check_fstat(suffix="")

    def test_fstat64(self):
        self._check_fstat(suffix="64")


if __name__ == "__main__":
    unittest.main()
