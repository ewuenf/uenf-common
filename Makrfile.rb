
Makr.loadExtension("ToolChainLinuxGcc")


# "parse" arguments
$arguments = Makr.getArgs()
$localDir = File.dirname($arguments.scriptFile)
$buildDir = $arguments.arguments[0]
$target = $arguments.arguments[1]

# set log level to info
Makr.log.level = Logger::INFO


$eigenDir = "/mnt/programmieren/math_libs/eigen"
$boostDir = "/mnt/programmieren/current_boost"


def configureBase()
  compilerConfig = $build.makeNewConfig("CompileTaskCPP")
  compilerConfig.clear()
  compilerConfig["compiler.cFlags"] = " -Wall -Werror -fexceptions"
  compilerConfig["compiler.includePaths"] = " -I#{$localDir}/src -I#{$eigenDir} -I#{$boostDir}"
end


def configureMaxDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -ggdb -fstack-check  -fno-eliminate-unused-debug-types"
  compilerConfig["compiler.defines"] = " -DDEBUG -D_GLIBCXX_DEBUG"
end


def configureStdLibDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -ggdb -fno-eliminate-unused-debug-types"
  compilerConfig["compiler.defines"] = " -DDEBUG -D_GLIBCXX_DEBUG"
end


def configureDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -ggdb -fno-eliminate-unused-debug-types"
  compilerConfig["compiler.defines"] = " -DDEBUG"
end


def configureReleaseStdLibDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -O2 -ggdb -fno-eliminate-unused-debug-types"
  compilerConfig["compiler.defines"] = " -D_GLIBCXX_DEBUG"
end


def configureReleaseDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -O2 -ggdb -fno-eliminate-unused-debug-types"
  compilerConfig["compiler.defines"] = " -DDEBUG"
end


def configureRelease()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -O3"
end


def configureProfiling()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] += " -O3 -pg"
end


def configure()

  configureBase()

  case $buildDir
    when "MaxDebug"
      configureMaxDebug()
    when "StdLibDebug"
      configureStdLibDebug()
    when "Debug"
      configureDebug()
    when "ReleaseStdLibDebug"
      configureReleaseStdLibDebug()
    when "ReleaseDebug"
      configureReleaseDebug()
    when "Release"
      configureRelease()
    when "Profiling"
      configureProfiling()
    else
      puts "unknown target, aborting"
      Makr.abortBuild()
  end
end


def buildIt()
  # first get build
  $build = Makr.loadBuild(Makr.cleanPathName($buildDir))
  # then use build block concept to ensure the build is saved
  $build.saveAfterBlock do
    configure()

    allCPPFiles = Makr::FileCollector.collect($localDir + "/src/", "*.{cpp,cxx}", true)
    tasks = Makr.applyGenerators(allCPPFiles, [Makr::CompileTaskGenerator.new($build, $build.getConfig("CompileTaskCPP"))])

    myLibTask = Makr.makeStaticLib($buildDir + "/libuenf-common.a", $build, tasks, $build.getConfig("CompileTaskCPP"))

    $build.build()
  end  
end


# main logic ###########################################################################################
if($target == "clean")
  system("rm -f " + $buildDir + "/*")
else
  buildIt()
end
