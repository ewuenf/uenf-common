
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
  compilerConfig["compiler"] = "g++"
  compilerConfig["compiler.includePaths"] = " -I" + $localDir + "/src -I/mnt/programmieren/math_libs/"
  compilerConfig["linker"] = "g++"
  compilerConfig["linker.libs"] = "-ljpeg -lboost_system -lboost_filesystem -lGL -lglut -lGLU -lgpcl -lraw1394"
  compilerConfig["linker.libs"] += " -ldc1394 -lz -lgpcl -lQtXml -lopencv_core -lopencv_calib3d"

  compilerConfigC = $build.makeNewConfig("CompileTaskC", "CompileTaskCPP")
  compilerConfigC["compiler"] = "gcc"
end


def configureMaxDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-ggdb -fstack-check  -fno-eliminate-unused-debug-types -Wall -fexceptions -Wno-long-long"
  compilerConfig["compiler.defines"] = "-DDEBUG -D_GLIBCXX_DEBUG"
end


def configureStdLibDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-ggdb -fno-eliminate-unused-debug-types -Wall -fexceptions -Wno-long-long"
  compilerConfig["compiler.defines"] = "-DDEBUG -D_GLIBCXX_DEBUG"
end


def configureDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-ggdb -fno-eliminate-unused-debug-types -Wall -fexceptions -Wno-long-long"
  compilerConfig["compiler.defines"] = "-DDEBUG"
end


def configureReleaseStdLibDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-O2 -ggdb -fno-eliminate-unused-debug-types -Wall -fexceptions -Wno-long-long"
  compilerConfig["compiler.defines"] = "-D_GLIBCXX_DEBUG"
end


def configureReleaseDebug()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-O2 -ggdb -fno-eliminate-unused-debug-types -Wall -fexceptions -Wno-long-long"
  compilerConfig["compiler.defines"] = "-DDEBUG"
end


def configureRelease()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-O3 -Wall -fexceptions -Wno-long-long"
end


def configureProfiling()
  compilerConfig = $build.getConfig("CompileTaskCPP")
  compilerConfig["compiler.cFlags"] = "-O3 -pg -Wall -fexceptions -Wno-long-long"
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



if($target == "clean")
  system("rm -f " + $buildDir + "/*")
else
  # first get build
  $build = Makr.loadBuild(Makr.cleanPathName($buildDir))
  # then use build block concept to ensure the build is saved
  $build.saveAfterBlock do
    configure()

    allCPPFiles = Makr::FileCollector.collect($localDir + "/src/", "*.{cpp,cxx}", true)
    tasks = Makr.applyGenerators(allCPPFiles, [Makr::CompileTaskGenerator.new($build, $build.getConfig("CompileTaskCPP"))])
    allHeaderFiles = Makr::FileCollector.collect($localDir + "/src/", "*.{h}", true)
    tasks.concat(Makr.applyGenerators(allHeaderFiles, [Makr::MocTaskGenerator.new($build, $build.getConfig("CompileTaskCPP"))]))
    allCFiles = Makr::FileCollector.collect($localDir + "/src/", "*.c", true)
    tasks.concat(Makr.applyGenerators(allCFiles,      [Makr::CompileTaskGenerator.new($build, $build.getConfig("CompileTaskC"))]))

    myProgramTask = Makr.makeProgram($buildDir + "/SIMERO_v2010.0", $build, tasks, $build.getConfig("CompileTaskCPP"))

    $build.nrOfThreads = 2
    $build.build()

    # make local link, so that the currently active configuration gets linked automatically
    File.delete("SIMERO_v2010.0") rescue nil
    File.symlink($buildDir + "/SIMERO_v2010.0", "SIMERO_v2010.0") rescue nil
  end
end
