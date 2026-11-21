#!/usr/bin/env python3
"""Local browser GUI for the assessed C++17 engine. Standard library only."""
import argparse
import csv
import io
import json
import mimetypes
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import threading
import time
import uuid
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse, parse_qs

ROOT = Path(__file__).resolve().parents[1]
STATIC = Path(__file__).resolve().parent / 'static'
BINARY = ROOT / 'build/social_cli'
FILES = {'users.csv', 'topics.csv', 'posts.csv', 'relationships.csv', 'interactions.csv'}
MAX_BODY = 12 * 1024 * 1024


class InputError(Exception):
    pass


def integer(value, lo, hi, label):
    if type(value) is not int or not lo <= value <= hi:
        raise InputError(f'{label} must be an integer from {lo:,} to {hi:,}.')
    return value


def read_csv(directory, name):
    with (directory / name).open(newline='', encoding='utf-8') as file:
        return list(csv.DictReader(file))


class Engine:
    def __init__(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='nikita-gui-')
        self.root = Path(self.temporary.name)
        self.datasets = {'sample': (ROOT / 'data/sample', 'Sample network', True)}
        self.reports = {}
        self.lock = threading.RLock()
        self.slots = threading.BoundedSemaphore(2)

    def close(self):
        self.temporary.cleanup()

    def run(self, *args, benchmark=False):
        binary = ROOT / 'build/social_bench' if benchmark else BINARY
        if not binary.is_file():
            raise InputError('Build the project first with make -j4.')
        if not self.slots.acquire(timeout=1):
            raise InputError('Two analyses are already running. Try again in a moment.')
        try:
            started = time.perf_counter()
            result = subprocess.run([str(binary), *map(str, args)], cwd=ROOT,
                                    capture_output=True, text=True, timeout=90)
            if result.returncode:
                raise InputError(result.stderr.strip().removeprefix('Error: ') or 'The C++ engine could not complete the request.')
            return result.stdout, round((time.perf_counter()-started)*1000, 2)
        except subprocess.TimeoutExpired:
            raise InputError('This instance exceeded 90 seconds. Try fewer users or a smaller budget.') from None
        finally:
            self.slots.release()

    def dataset(self, key):
        if not isinstance(key,str):
            raise InputError('Dataset ID must be a string.')
        with self.lock:
            if key not in self.datasets:
                raise InputError('Dataset not found. Load or generate it again.')
            return self.datasets[key]

    def register(self, directory, name, synthetic=False):
        if not all((directory / filename).is_file() for filename in FILES):
            raise InputError('Choose all five CSV files: users, topics, posts, relationships and interactions.')
        if len(read_csv(directory, 'users.csv')) > 10000:
            raise InputError('The interactive GUI supports up to 10,000 users. Use the CLI for larger datasets.')
        # Use the C++ loader as the authoritative schema/reference validator.
        with tempfile.TemporaryDirectory(dir=self.root) as temp:
            self.run('analyze', directory, 0, Path(temp)/'validation')
        with self.lock:
            if len(self.datasets) >= 50:
                raise InputError('This session has 50 datasets. Restart the GUI to clear them.')
            key = uuid.uuid4().hex
            self.datasets[key] = (directory, name[:120], synthetic)
        return {'dataset': key, 'name': name[:120], 'synthetic': synthetic}

    def generate(self, data):
        count = integer(data.get('users'), 2, 10000, 'User count')
        directory = self.root / ('dataset-'+uuid.uuid4().hex)
        self.run('generate', count, directory)
        return self.register(directory, f'Generated · {count:,} users', True)

    def load(self, data):
        if 'files' in data:
            files = data['files']
            if not isinstance(files, dict) or set(files) != FILES or not all(isinstance(v,str) for v in files.values()):
                raise InputError('Select exactly the five required CSV files.')
            directory = self.root / ('upload-'+uuid.uuid4().hex)
            directory.mkdir()
            for name, content in files.items():
                (directory/name).write_text(content, encoding='utf-8')
            return self.register(directory, 'Imported CSV network')
        path = data.get('path')
        if not isinstance(path, str) or not path.strip():
            raise InputError('Enter the CSV dataset directory.')
        directory = Path(path).expanduser()
        if not directory.is_absolute():
            directory = ROOT / directory
        if not directory.is_dir():
            raise InputError('That directory does not exist.')
        # Snapshot all inputs so later external edits do not change an analyzed dataset.
        if not all((directory/name).is_file() for name in FILES):
            raise InputError('The directory must contain all five CSV files.')
        if sum((directory/name).stat().st_size for name in FILES) > MAX_BODY:
            raise InputError('Dataset files must total at most 12 MB in the GUI.')
        snapshot = self.root / ('loaded-'+uuid.uuid4().hex)
        snapshot.mkdir()
        for name in FILES:
            shutil.copyfile(directory/name, snapshot/name)
        return self.register(snapshot, directory.name)

    @staticmethod
    def scenario(data):
        scenario = data.get('scenario', 'normal')
        if scenario not in ('normal', 'shock'):
            raise InputError('Scenario must be normal or shock.')
        return scenario

    def analyze(self, data):
        key = data.get('dataset', 'sample')
        directory, name, synthetic = self.dataset(key)
        scenario = self.scenario(data)
        budget = integer(data.get('budget'), 0, 1000000, 'Budget')
        token = uuid.uuid4().hex
        prefix = self.root / ('report-'+token)
        args = ['analyze', directory, budget, prefix]
        if scenario == 'shock':
            args.append('shock')
        log, elapsed = self.run(*args)
        report = json.loads(prefix.with_suffix('.json').read_text())
        users = read_csv(directory, 'users.csv')
        costs = {int(u['id']): int(u['campaign_cost']) for u in users}
        for row in report['rankings']:
            row['cost'] = costs[row['id']]
        candidates = report['rankings'][:report['campaign_candidate_count']]
        shown = {r['id'] for r in report['rankings'][:80]}
        shown.update(report['campaign_users'])
        shown.update((int(users[0]['id']), int(users[-1]['id'])))
        links = read_csv(directory, 'relationships.csv')
        edges, active = [], 0
        for row in links:
            u,v,delay,capacity = (int(row[k]) for k in ('from','to','delay','capacity'))
            if scenario == 'shock':
                capacity //= 4
            if capacity > 0:
                active += 1
                if u in shown and v in shown and len(edges) < 1000:
                    edges.append({'from':u,'to':v,'delay':delay,'capacity':capacity})
        topics = read_csv(directory, 'topics.csv')
        topic_names = {int(t['id']): t['name'] for t in topics}
        for assignment in report['creator_topic_assignment']:
            assignment['topic_name'] = topic_names[assignment['topic']]
        report.update({
            'dataset': key, 'dataset_name': name, 'synthetic': synthetic,
            'scenario': scenario, 'elapsed_ms': elapsed, 'run_id': token,
            'sink_user': int(users[-1]['id']), 'relationships': len(links), 'active_relationships': active,
            'posts': len(read_csv(directory, 'posts.csv')), 'topic_count':len(topics),
            'interactions':len(read_csv(directory, 'interactions.csv')),
            'campaign_cost':sum(costs[u] for u in report['campaign_users']),
            'candidates':candidates, 'graph_nodes':[r for r in report['rankings'] if r['id'] in shown],
            'graph_edges':edges, 'graph_sampled':len(shown)<report['users'], 'log':log,
        })
        with self.lock:
            if len(self.reports) >= 100:
                old = next(iter(self.reports))
                old_prefix = self.reports.pop(old)
                for suffix in ('.json','.csv'):
                    old_prefix.with_suffix(suffix).unlink(missing_ok=True)
            self.reports[token] = prefix
        return report

    def route(self, data):
        directory, _, _ = self.dataset(data.get('dataset','sample'))
        source = integer(data.get('source'), 0, 1000000000, 'Source ID')
        target = integer(data.get('target'), 0, 1000000000, 'Target ID')
        method = data.get('method')
        if method not in ('dijkstra','bellman','floyd'):
            raise InputError('Choose Dijkstra, Bellman-Ford or Floyd-Warshall.')
        ids = {int(user['id']) for user in read_csv(directory, 'users.csv')}
        if source not in ids or target not in ids:
            raise InputError('Source and target IDs must belong to the current dataset.')
        scenario = self.scenario(data)
        if scenario == 'normal':
            log, elapsed = self.run('route',directory,source,target,method)
        else:
            with tempfile.TemporaryDirectory(dir=self.root) as temp:
                temp = Path(temp)
                for name in FILES - {'relationships.csv'}:
                    shutil.copyfile(directory/name, temp/name)
                links = read_csv(directory,'relationships.csv')
                with (temp/'relationships.csv').open('w',newline='') as file:
                    writer = csv.DictWriter(file,fieldnames=['from','to','delay','capacity'])
                    writer.writeheader()
                    for row in links:
                        row['capacity'] = int(row['capacity'])//4
                        writer.writerow(row)
                log, elapsed = self.run('route',temp,source,target,method)
        return {'output': log.strip(), 'elapsed_ms': elapsed, 'method':method,'scenario':scenario}

    def match(self, data):
        method, text, pattern = data.get('method'), data.get('text'), data.get('pattern')
        if method not in ('naive','rk','fa','kmp'):
            raise InputError('Unknown string matching strategy.')
        if not isinstance(text,str) or not isinstance(pattern,str) or '\0' in text+pattern:
            raise InputError('Text and pattern must be strings without null bytes.')
        if len(text.encode())>60000 or len(pattern.encode())>4000:
            raise InputError('Use text up to 60,000 bytes and a pattern up to 4,000 bytes.')
        log, elapsed = self.run('match',method,text,pattern)
        return {'positions':[int(x) for x in log.split()], 'elapsed_ms':elapsed,'method':method}


class Server(ThreadingHTTPServer):
    daemon_threads = True
    def __init__(self, address, engine):
        self.engine = engine
        super().__init__(address, Handler)


class Handler(BaseHTTPRequestHandler):
    server_version = 'InfluenceStudio/1.0'
    def log_message(self, fmt, *args):
        print(f'[GUI] {self.address_string()} {fmt % args}', flush=True)

    def send(self, status, body, content_type='application/json; charset=utf-8', filename=None):
        if isinstance(body,dict):
            body = json.dumps(body,ensure_ascii=False).encode()
        self.send_response(status)
        self.send_header('Content-Type',content_type)
        self.send_header('Content-Length',str(len(body)))
        self.send_header('X-Content-Type-Options','nosniff')
        self.send_header('Referrer-Policy','same-origin')
        self.send_header('Cache-Control','no-store')
        self.send_header('Content-Security-Policy',"default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'")
        if filename:
            self.send_header('Content-Disposition',f'attachment; filename="{filename}"')
        self.end_headers()
        self.wfile.write(body)

    def trusted_request(self):
        port = self.server.server_address[1]
        allowed = {f'127.0.0.1:{port}', f'localhost:{port}'}
        if self.headers.get('Host') not in allowed:
            raise InputError('Open this GUI through its local address.')
        origin = self.headers.get('Origin')
        if origin and origin not in {'http://'+host for host in allowed}:
            raise InputError('Cross-origin requests are not allowed.')

    def do_GET(self):
        try:
            self.trusted_request()
            parsed = urlparse(self.path)
            if parsed.path == '/api/status':
                return self.send(200,{'ready':BINARY.is_file(),'dataset':'sample','name':'Sample network'})
            if parsed.path == '/api/download':
                args = parse_qs(parsed.query)
                token = args.get('run',[''])[0]
                fmt = args.get('format',[''])[0]
                if fmt not in ('csv','json') or not re.fullmatch('[a-f0-9]{32}',token):
                    raise InputError('Invalid report download.')
                with self.server.engine.lock:
                    prefix = self.server.engine.reports.get(token)
                if prefix is None:
                    return self.send(404,{'error':'Report expired. Run the analysis again.'})
                return self.send(200,prefix.with_suffix('.'+fmt).read_bytes(),
                                 'text/csv; charset=utf-8' if fmt=='csv' else 'application/json',
                                 'network-analysis.'+fmt)
            assets = {'/':'index.html','/index.html':'index.html','/app.js':'app.js','/style.css':'style.css','/favicon.svg':'favicon.svg'}
            if parsed.path not in assets:
                return self.send(404,{'error':'Not found.'})
            path = STATIC/assets[parsed.path]
            return self.send(200,path.read_bytes(),mimetypes.guess_type(path)[0] or 'application/octet-stream')
        except InputError as error:
            self.send(400,{'error':str(error)})
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_POST(self):
        try:
            self.trusted_request()
            if self.headers.get('Content-Type','').split(';')[0] != 'application/json':
                raise InputError('Use JSON request data.')
            try:
                length = int(self.headers.get('Content-Length','0'))
            except ValueError:
                raise InputError('Invalid request length.') from None
            if not 0 < length <= MAX_BODY:
                raise InputError('Request must be between 1 byte and 12 MB.')
            try:
                data = json.loads(self.rfile.read(length))
            except (ValueError,UnicodeError):
                raise InputError('Invalid JSON request.') from None
            if not isinstance(data,dict):
                raise InputError('Request data must be an object.')
            engine = self.server.engine
            actions = {'/api/generate':engine.generate, '/api/load':engine.load,
                       '/api/analyze':engine.analyze,'/api/route':engine.route,'/api/match':engine.match}
            if self.path == '/api/workbench':
                log, elapsed = engine.run('workbench')
                return self.send(200,{'output':log,'elapsed_ms':elapsed})
            if self.path == '/api/benchmark':
                with tempfile.TemporaryDirectory(dir=engine.root) as temp:
                    path = Path(temp)/'benchmarks.csv'
                    _, elapsed = engine.run(path,benchmark=True)
                    return self.send(200,{'csv':path.read_text(),'elapsed_ms':elapsed,'cases':122})
            if self.path not in actions:
                return self.send(404,{'error':'Unknown action.'})
            self.send(200,actions[self.path](data))
        except InputError as error:
            self.send(400,{'error':str(error)})
        except (OSError,KeyError,ValueError,csv.Error,UnicodeError) as error:
            self.send(400,{'error':f'Could not read the dataset: {error}'})
        except (BrokenPipeError,ConnectionResetError):
            pass
        except Exception:
            self.log_error('Unexpected GUI server error')
            self.send(500,{'error':'The request could not be completed. Check the terminal and retry.'})


def main():
    parser = argparse.ArgumentParser(description='Start the local Influence Studio GUI')
    parser.add_argument('--port',type=int,default=8765)
    parser.add_argument('--no-browser',action='store_true')
    args = parser.parse_args()
    if not 0 <= args.port <= 65535:
        parser.error('port must be 0..65535')
    if not BINARY.is_file():
        result = subprocess.run(['make','-j4','all'],cwd=ROOT)
        if result.returncode:
            raise SystemExit(result.returncode)
    engine = Engine()
    try:
        server = Server(('127.0.0.1',args.port),engine)
    except OSError as error:
        engine.close()
        raise SystemExit(f'Cannot start GUI: {error}. Choose another port with --port 8766.') from None
    url = f'http://127.0.0.1:{server.server_address[1]}'
    print(f'\nInfluence Studio is running at {url}\nPress Ctrl+C to stop.\n',flush=True)
    if not args.no_browser:
        threading.Timer(.5,lambda:webbrowser.open(url)).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print('\nGUI stopped.')
    finally:
        server.server_close()
        engine.close()


if __name__ == '__main__':
    main()
