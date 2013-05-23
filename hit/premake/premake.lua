-- target 是预定义变量,表示编译环境的名字
project.path = "../build/" .. target
project.name = "hit"
project.bindir = "../bin"
project.libdir = "../lib"

-----------------------------
-- Antbuster package
-----------------------------
package = newpackage()

package.path = "../build/" .. target
package.kind = "winexe"
package.name = "hit"
package.language = "c++"
package.bindir = "../../bin"

package.config["Debug"].objdir = "./Debug"
package.config["Debug"].target = package.name .. "_d"
package.config["Release"].objdir = "./Release"
package.config["Release"].target = package.name

package.linkoptions ={ "/NODEFAULTLIB:libc" }
package.buildflags = {"no-main", "extra-warnings", "static-runtime", "no-exceptions", "no-rtti" }
package.config["Debug"].links = { "hge", "hgehelp", "curvedani_d", "hgeport_d" }
package.config["Release"].links = { "hge", "hgehelp", "curvedani", "hgeport" }
package.includepaths = { "../../include", "../../include/ca", "../../include/hge" }

package.libpaths = { "../../lib", "../../lib/" .. target } 

package.files = {
  matchrecursive("../../include/*.h", "../../src/*.cpp", "../../src/*.h")
}
