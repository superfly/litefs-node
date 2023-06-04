import * as fs from 'fs'

const addon = require('../build/Release/litefs-node-native');

const LiteFS = addon.LitefsNode

export function WithHalt(databasePath: string, func: () => Promise<any>): Promise<any> {
  let fd = 0;
  try {
    fd = fs.openSync(databasePath + '-lock','a+', 0o666);
    LiteFS.Halt(fd);
    return Promise.resolve(func()).finally(() => LiteFS.Unhalt(fd));
  } finally {
    if (fd) fs.closeSync(fd);
  }
}

