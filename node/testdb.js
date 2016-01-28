var sqlite3 = require('sqlite3').verbose();
var db = new sqlite3.Database('porter.db');

db.serialize(function () {
    db.run("CREATE TABLE IF NOT EXISTS porter (id INTEGER PRIMARY KEY ASC, action TEXT, status TEXT, time TIMESTAMP, humanid TEXT)");

    function insertLoop() {
        for (var i = 0; i < 10; i++) {
            db.run("INSERT INTO porter (action, status, time, humanid) VALUES ('ENTRANCE', 'VENKOVNI', ?, '098765')", '2015-09-15T13:33:0'+i);
        }
    }

    insertLoop();
    db.run("INSERT INTO porter (action, status, time, humanid) VALUES ('ENTRANCE', 'VNITRNI', datetime('now'), '098765')");

    db.each("SELECT * FROM porter", function (err, row) {
        console.log("This app was run at " + row.action + "| " + row.status + "|" + row.time + "|" + row.humanid);
    });
});

db.close();