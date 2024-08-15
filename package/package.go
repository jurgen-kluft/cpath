package cpath

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	ctime "github.com/jurgen-kluft/ctime/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
	cvmem "github.com/jurgen-kluft/cvmem/package"
)

// GetPackage returns the package object of 'cpath'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	ccorepkg := ccore.GetPackage()
	cbasepkg := cbase.GetPackage()
	cvmempkg := cvmem.GetPackage()
	ctimepkg := ctime.GetPackage()

	// The main (cpath) package
	mainpkg := denv.NewPackage("cpath")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(ccorepkg)
	mainpkg.AddPackage(cbasepkg)
	mainpkg.AddPackage(cvmempkg)
	mainpkg.AddPackage(ctimepkg)

	// 'cpath' library
	mainlib := denv.SetupDefaultCppLibProject("cpath", "github.com\\jurgen-kluft\\cpath")
	mainlib.Dependencies = append(mainlib.Dependencies, ccorepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, cvmempkg.GetMainLib())
	mainlib.Dependencies = append(mainlib.Dependencies, ctimepkg.GetMainLib())

	// 'cpath' unittest project
	maintest := denv.SetupDefaultCppTestProject("cpath"+"_test", "github.com\\jurgen-kluft\\cpath")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, ccorepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cvmempkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, ctimepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
