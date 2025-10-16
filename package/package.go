package cpath

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	ctime "github.com/jurgen-kluft/ctime/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
	cvmem "github.com/jurgen-kluft/cvmem/package"
)

const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "cpath"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	cunittestpkg := cunittest.GetPackage()
	cbasepkg := cbase.GetPackage()
	ctimepkg := ctime.GetPackage()
	cvmempkg := cvmem.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cbasepkg)
	mainpkg.AddPackage(ctimepkg)
	mainpkg.AddPackage(cvmempkg)

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	mainlib.AddDependencies(cbasepkg.GetMainLib())
	mainlib.AddDependencies(ctimepkg.GetMainLib())
	mainlib.AddDependencies(cvmempkg.GetMainLib())

	// test library
	testlib := denv.SetupCppTestLibProject(mainpkg, name)
	testlib.AddDependencies(cbasepkg.GetTestLib())
	testlib.AddDependencies(ctimepkg.GetTestLib())
	testlib.AddDependencies(cvmempkg.GetTestLib())
	testlib.AddDependencies(cunittestpkg.GetTestLib())

	// unittest project
	maintest := denv.SetupCppTestProject(mainpkg, name)
	maintest.AddDependencies(cunittestpkg.GetMainLib())
	maintest.AddDependency(testlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddTestLib(testlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
