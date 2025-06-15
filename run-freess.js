const freessModule = require('./freess.js');

const args = process.argv.slice(2); // skip "node" and script name

freessModule().then((Module) => {
  Module.onRuntimeInitialized = () => {
    try {
      // Set up virtual FS from local directory
      Module.FS.mkdir('/working');
      Module.FS.mount(Module.FS.filesystems.NODEFS, { root: '.' }, '/working');
      Module.FS.chdir('/working');

      // Call main with command-line arguments
      Module.callMain(args);
    } catch (e) {
      console.error("Runtime error:", e);
    }
  };
});

