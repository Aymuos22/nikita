"""Exercise the GUI HTTP bridge against the real C++ engine (stdlib only)."""
import csv
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import unittest
from urllib.error import HTTPError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'gui'))
from server import Engine, Server


class GUITests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.engine = Engine()
        cls.server = Server(('127.0.0.1',0), cls.engine)
        cls.thread = threading.Thread(target=cls.server.serve_forever,daemon=True)
        cls.thread.start()
        cls.url = f'http://127.0.0.1:{cls.server.server_address[1]}'

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join()
        cls.engine.close()

    def request(self,path,data=None,headers=None):
        request = Request(self.url+path,data=None if data is None else json.dumps(data).encode(),
                          headers=headers or ({'Content-Type':'application/json'} if data is not None else {}))
        try:
            with urlopen(request,timeout=100) as response:
                return response.status,response.read(),response.headers
        except HTTPError as error:
            return error.code,error.read(),error.headers

    def post(self,path,data):
        status,body,_=self.request(path,data)
        self.assertEqual(status,200,body.decode())
        return json.loads(body)

    def test_static_and_status(self):
        for path in ('/','/app.js','/style.css','/favicon.svg'):
            status,body,headers=self.request(path)
            self.assertEqual(status,200)
            self.assertTrue(body)
            self.assertIn('Content-Security-Policy',headers)
        status,body,_=self.request('/api/status')
        self.assertEqual(status,200)
        self.assertTrue(json.loads(body)['ready'])
        self.assertEqual(self.request('/../../Makefile')[0],404)

    def test_sample_scenarios_and_exports(self):
        for scenario,budget,score,flow,communities in [('normal',35,472,36,1),('shock',10,265,8,2),('normal',0,0,36,1)]:
            report=self.post('/api/analyze',{'dataset':'sample','budget':budget,'scenario':scenario})
            self.assertEqual((report['campaign_value'],report['max_flow'],report['communities']),(score,flow,communities))
            self.assertEqual(report['users'],12)
            self.assertEqual(len(report['rankings']),12)
            self.assertLessEqual(report['campaign_cost'],budget)
            self.assertEqual(len(report['creator_topic_assignment']),3)
            self.assertEqual(len(report['graph_nodes']),12)
            for edge in report['graph_edges']:
                self.assertGreater(edge['capacity'],0)
            status,body,headers=self.request(f'/api/download?run={report["run_id"]}&format=json')
            self.assertEqual(status,200)
            self.assertIn('attachment',headers['Content-Disposition'])
            self.assertEqual(json.loads(body)['campaign_value'],score)
            status,body,_=self.request(f'/api/download?run={report["run_id"]}&format=csv')
            self.assertEqual(status,200)
            csv_rows=list(csv.DictReader(io.StringIO(body.decode())))
            self.assertEqual(len(csv_rows),12)
            self.assertEqual([int(r['id']) for r in csv_rows],[r['id'] for r in report['rankings']])

    def test_generated_stress(self):
        generated=self.post('/api/generate',{'users':10000})
        report=self.post('/api/analyze',{'dataset':generated['dataset'],'budget':35})
        self.assertEqual(report['users'],10000)
        self.assertEqual(report['campaign_value'],1233)
        self.assertIsNone(report['max_flow'])
        self.assertEqual(len(report['rankings']),10000)
        self.assertTrue(report['graph_sampled'])
        self.assertLessEqual(len(report['graph_nodes']),102)
        self.assertLessEqual(len(report['graph_edges']),1000)

    def test_import_and_snapshot(self):
        files={name:(ROOT/'data/sample'/name).read_text() for name in ['users.csv','topics.csv','posts.csv','relationships.csv','interactions.csv']}
        imported=self.post('/api/load',{'files':files})
        report=self.post('/api/analyze',{'dataset':imported['dataset'],'budget':35})
        self.assertEqual(report['campaign_value'],472)
        self.assertFalse(report['synthetic'])
        with tempfile.TemporaryDirectory() as temp:
            directory=Path(temp)
            for name,text in files.items():
                (directory/name).write_text(text)
            loaded=self.post('/api/load',{'path':str(directory)})
            (directory/'users.csv').unlink()
            report=self.post('/api/analyze',{'dataset':loaded['dataset'],'budget':35})
            self.assertEqual(report['campaign_value'],472)
        # Byte LCS can be invalid UTF-8 even when both inputs are valid Unicode.
        # The workbench reports its byte length, never undecodable raw bytes.
        unicode_files=dict(files)
        unicode_files['posts.csv']=files['posts.csv'].replace('community technology campaign post 0','é').replace('community technology campaign post 1','ê')
        unicode_dataset=self.post('/api/load',{'files':unicode_files})
        unicode_workbench=self.post('/api/workbench',{'dataset':unicode_dataset['dataset'],'budget':35})
        self.assertIn("LCS first two users' post text byte length=1",unicode_workbench['output'])
        # Malformed input is rejected by the authoritative C++ loader.
        files['relationships.csv']+='99999,0,1,1\n'
        self.assertEqual(self.request('/api/load',{'files':files})[0],400)

    def test_routing_and_matching(self):
        for scenario in ('normal','shock'):
            outputs=[]
            for method in ('dijkstra','bellman','floyd'):
                result=self.post('/api/route',{'dataset':'sample','source':0,'target':11,'method':method,'scenario':scenario})
                outputs.append(result['output'].split(';')[0])
                self.assertEqual(result['scenario'],scenario)
            self.assertEqual(len(set(outputs)),1)
            self.assertEqual(outputs[0],'delay=6' if scenario=='normal' else 'delay=23')
        for method in ('naive','rk','fa','kmp'):
            for text,pattern,positions in [('aaaaa','aaa',[0,1,2]),('abc','',[0,1,2,3]),('café café','café',[0,6])]:
                result=self.post('/api/match',{'method':method,'text':text,'pattern':pattern})
                self.assertEqual(result['positions'],positions)

    def test_workbench_and_benchmarks(self):
        result=self.post('/api/workbench',{})
        self.assertIn('CRT counter reconstruction=45',result['output'])
        result=self.post('/api/benchmark',{})
        rows=list(csv.DictReader(io.StringIO(result['csv'])))
        self.assertEqual(len(rows),144)
        self.assertEqual(len({r['algorithm'] for r in rows}),36)
        self.assertTrue(all(int(r['measured_peak_heap_bytes'])>=0 for r in rows))
        adversarial={r['algorithm']:r for r in rows if r['dataset']=='bounded_adversarial'}
        self.assertEqual(float(adversarial['knapsack_dp']['solution_value']),220)
        self.assertEqual(float(adversarial['fractional_knapsack']['solution_value']),240)
        self.assertGreater(float(adversarial['fractional_knapsack']['relative_gap']),0)
        generated=self.post('/api/generate',{'users':7})
        workbench=self.post('/api/workbench',{'dataset':generated['dataset'],'budget':10,'scenario':'shock'})
        self.assertIn('DATASET WORKBENCH: 7 users',workbench['output'])
        self.assertIn('Scenario=capacity_shock; budget=10',workbench['output'])

    def test_same_input_comparisons(self):
        for scenario,budget,score,distance in [('normal',35,472,6),('shock',10,265,23)]:
            result=self.post('/api/compare',{'dataset':'sample','budget':budget,'scenario':scenario})
            rows={r['algorithm']:r for r in result['rows']}
            self.assertEqual(float(rows['knapsack_dp']['solution_value']),score)
            self.assertEqual(float(rows['dijkstra']['solution_value']),distance)
            self.assertGreater(int(rows['knapsack_dp']['measured_peak_heap_bytes']),0)
            for row in rows.values():
                if row['quality_kind'].startswith('exact_'):
                    self.assertEqual(float(row['relative_gap']),0)
            self.assertEqual(result['scenario'],scenario)
        files={'users.csv':'id,name,campaign_cost\n0,A,1\n1,B,1\n',
               'topics.csv':'id,name\n','posts.csv':'id,user_id,topic_id,text\n',
               'relationships.csv':'from,to,delay,capacity\n',
               'interactions.csv':'from,to,post_id,timestamp,strength\n'}
        imported=self.post('/api/load',{'files':files})
        report=self.post('/api/compare',{'dataset':imported['dataset'],'budget':0})
        unreachable={r['algorithm']:r for r in report['rows']}['dijkstra']
        self.assertEqual(unreachable['quality_kind'],'unreachable')
        self.assertEqual(unreachable['solution_value'],'')

    def test_validation_and_origin_protection(self):
        cases=[('/api/analyze',{'budget':-1}),('/api/analyze',{'budget':1.5}),
               ('/api/analyze',{'budget':True}),('/api/analyze',{'dataset':{},'budget':35}),
               ('/api/analyze',{'dataset':'missing','budget':35}),('/api/analyze',{'budget':35,'scenario':'typo'}),
               ('/api/generate',{'users':10001}),('/api/generate',{'users':1}),
               ('/api/load',{'path':'missing-directory'}),('/api/load',{'files':{'../file':'x'}}),
               ('/api/route',{'source':0,'target':999,'method':'dijkstra'}),
               ('/api/route',{'source':0,'target':11,'method':'typo'}),
               ('/api/match',{'method':'kmp','text':'a\0','pattern':'a'}),
               ('/api/match',{'method':'bad','text':'abc','pattern':'a'})]
        for path,data in cases:
            with self.subTest(path=path,data=data):
                self.assertEqual(self.request(path,data)[0],400)
        self.assertEqual(self.request('/api/status',headers={'Host':'evil.example'})[0],400)
        self.assertEqual(self.request('/api/analyze',{'budget':35},headers={'Content-Type':'application/json','Origin':'http://evil.example'})[0],400)
        self.assertEqual(self.request('/api/analyze',{'budget':35},headers={'Content-Type':'text/plain'})[0],400)
        self.assertEqual(self.request('/api/download?run=../../secret&format=txt')[0],400)
        self.assertEqual(self.request('/api/download?run='+'a'*32+'&format=json')[0],404)


if __name__ == '__main__':
    unittest.main(verbosity=2)
