def depends(ctx):
    pass


def options(opt):
    opt.load("compiler_cxx")
    opt.load("boost")


def configure(conf):
    conf.load("compiler_cxx")
    conf.load("boost")
    conf.check_cxx(
            uselib_store="LIBUSB4FLYSPIRWAPI",
            mandatory=True,
            header_name="libusb-1.0/libusb.h",
            lib="usb-1.0")


def build(bld):
    bld(
            target="flyspi-rw_api_inc",
            export_includes=["include"])
    bld.shlib(
            target="flyspi-rw_api",
            source=[
                    "src/flyspi_com.cpp",
                    "src/usb_communication.cpp"],
            features="cxx",
            use=["flyspi-rw_api_inc", "LIBUSB4FLYSPIRWAPI"])
    bld.program(
            target="flyspi-rw_api_test",
            source=["test/example-user.cpp"],
            use=["flyspi-rw_api"])
