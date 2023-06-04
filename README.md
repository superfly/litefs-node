LiteFS Node.js Library
=================

This Node.js library is for interacting with LiteFS features that cannot be accessed
through the typical SQLite API.


## Halting

LiteFS provides the ability to halt writes on the primary node in order that
replicas may execute writes remotely and forward them back to the primary. This
isn't necessary in most usage, however, it can make running migrations simpler.

Write forwarding from the replica is much slower than executing the write
transaction directly on the primary so only use this for migrations or low-write
scenarios.

```javascript
import sqlite3 from 'sqlite3';
import { WithHalt } from '@flydotio/litefs';

const db = new sqlite3.Database("/litefs/my.db");

const count = 1;

// Execute a write transaction from any node.
// If this is a replica, it will run the inner function with the HALT lock.
(async () => {
  await WithHalt("/litefs/my.db", async () => {
    await new Promise(resolve, reject) => {
      db.run('INSERT INTO "welcome" VALUES(?)', [count], err => {
	err ? reject(err) : resolve()
      })
    })
  }
})();
```
