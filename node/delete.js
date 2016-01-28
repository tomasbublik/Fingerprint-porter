var sqlite3 = require('sqlite3').verbose();
var db = new sqlite3.Database('porter.db');

db.serialize(function () {
    db.run("DELETE FROM porter");
    db.each("SELECT * FROM porter", function (err, row) {
        console.log("This app was run at " + row.action + "| " + row.status + "|" + row.time + "|" + row.humanid);
    });
});

db.close();