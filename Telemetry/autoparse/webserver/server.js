const app 		    = require('express')();
const bodyparser 	= require('body-parser');
const busboy        = require('connect-busboy');
const child_process = require('child_process');
const fs			= require('fs');
const path          = require('path');

const PORT          = 5000;
const UPLOAD_PATH	= './uploads';

app.use(bodyparser.urlencoded({ extended: true }));
app.use(busboy({ highWaterMark: 2 * 1024 * 1024 }));
fs.mkdirSync(UPLOAD_PATH)

app.get('/', (req, res) => res.sendFile(path.resolve(__dirname, 'views/index.html')));
app.get('/upload', (req, res) => res.sendFile(path.resolve(__dirname, 'views/upload.html')));
app.get('/status', (req, res) => res.sendFile(path.resolve(__dirname, 'views/status.html')))

app.post('/upload', (req, res) => {
	const db_name = req.body.db_name.trim().replace(/ /g,"_");
	if (!db_name) return res.redirect('/upload?err=true');

	req.pipe(req.busboy); 
    req.busboy.on('file', (fieldname, file, filename) => {
		console.log(`Upload of '${filename}' started`);
		
		const filepath = path.join(UPLOAD_PATH, filename);
        const fstream = fs.createWriteStream(filepath);
        file.pipe(fstream);
 
        fstream.on('close', () => {
            console.log(`Upload of '${filename}' finished`);
			res.send(`Your CSV is being written to database ${db_name}. Go to <a href=http://ec2-3-134-2-166.us-east-2.compute.amazonaws.com:3000/>Grafana</a> to view`);

			child_process.exec('../exe/upload ' + db_name + ' ' + filepath, () => fs.rm(filename))
        });
    });
});

// --- Everything past this point is related to configuration management --- //

const runScript = script => child_process.execSync('sh ~/update_scripts/' + script).toString()
const getScriptOutput = script => runScript(script).split('\n').map(x => x.trim()).filter(x => x.length);
const getBranches = () => getScriptOutput('branches.sh')
const restart = branch => getBranches().includes(branch) && runScript('restart.sh ' + branch).toString();

app.get('/config', (req, res) => { let [ status, current ] = getScriptOutput('status.sh'); res.json({ status, current, parserIDs: getScriptOutput('list_canIDs.sh'), branches: getBranches() }); })
app.post('/refresh', (req, res) => { restart(req.body.new_branch); res.redirect('/status'); })

app.listen(PORT, () => console.log("Grafana upload server running on port " + PORT));
