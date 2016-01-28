var sqlite3 = require('sqlite3').verbose();
var db = new sqlite3.Database('porter.db');

var express = require('express');
var app = express();
var MAX_NUMBER_OF_ROWS = 100;

async = require("async");

console.log("Registering endpoint: /");
app.get('/', function (req, res) {
    res.send('hello ROOT world');
});

app.get('/data', function (req, res) {
    var dateFrom = req.query.dateFrom;
    console.log("What is param dateFrom?: " + dateFrom);
    var dateTo = req.query.dateTo;
    console.log("What is param dateTo?: " + dateTo);
    //to get parameter 'sourceid':req.query.sourceid
    var result = [];

    var finishRequest = function (result) {
        res.contentType('application/json');
        res.send(JSON.stringify(result));
    };
    var selectAll = "SELECT *";
    var selectCount = "SELECT COUNT(*) AS actionsCount";
    var from = " FROM porter";
    var limit = " LIMIT ";
    var where = "";

    if (dateFrom != null) {
        where += " WHERE datetime(time)";
        where += " >= datetime('" + dateFrom + " 00:00:00')";
        if (dateTo != null) {
            where += " and datetime(time) <= datetime('" + dateTo + " 23:59:59')";
        }
    }

    if (dateTo != null && dateFrom == null) {
        where += " WHERE datetime(time)";
        where += " <= datetime('" + dateTo + " 23:59:59')";
    }

    var selectQuery = selectAll + from + where + limit + MAX_NUMBER_OF_ROWS;
    console.log("Running query: " + selectQuery);
    db.all(selectQuery, function (err, rows) {
        console.log("Results number: " + rows.length);
        rows.forEach(eachRow);
        var selecCountQeury = selectCount + from + where;
        console.log("Running query: " + selecCountQeury);
        db.get(selecCountQeury, function (err, rows) {
            var actionsCount = rows.actionsCount;
            console.log("Real count is: " + actionsCount);
            if (actionsCount > result.length) {
                result.push({
                    "akce": "Zobrazuje se pouze " + MAX_NUMBER_OF_ROWS + ", nicméně výsledků je: " + actionsCount,
                    "stav": "",
                    "time": "",
                    "id": ""
                });
            }
            finishRequest(result);
        });
    });

    function eachRow(row, i, rows) {
        //console.log(row.time+" -0200");
        result.push({
            "akce": row.action,
            "stav": row.status,
            "time": row.time,
            //"time": Date.parse(row.time),
            //"time": Date.parse(row.time+" -0200"),
            "id": row.humanid
        });
    }
});

var port = 1337;
app.listen(port, '0.0.0.0');
