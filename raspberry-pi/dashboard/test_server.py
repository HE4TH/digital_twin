import json
import threading
import unittest
import urllib.request
import urllib.error
from http.server import ThreadingHTTPServer
from unittest.mock import patch
from server import State, handler


class DashboardTests(unittest.TestCase):
    def test_quest_stream_demand_limits_and_expiry(self):
        state=State();http=ThreadingHTTPServer(('127.0.0.1',0),handler(state,'secret',0))
        port=http.server_port;http.RequestHandlerClass=handler(state,'secret',port)
        thread=threading.Thread(target=http.serve_forever,daemon=True);thread.start()
        url=f'http://127.0.0.1:{port}'
        headers={'X-Quest-Stream':'1','Content-Type':'image/jpeg'}
        try:
            request=urllib.request.Request(url+'/api/quest/demand',headers=headers)
            with urllib.request.urlopen(request) as response:self.assertEqual(response.read(),b'0')
            with urllib.request.urlopen(url+'/api/state') as response:self.assertTrue(json.load(response)['quest']['connected'])
            with urllib.request.urlopen(request) as response:self.assertEqual(response.read(),b'1')
            for payload,changed,code in [(b'not jpeg',{},400),(b'\xff\xd8\xff\xd9',{'X-Quest-Stream':'0'},403),
                                         (b'\xff\xd8\xff\xd9',{'Origin':'https://evil.example'},403),
                                         (b'x'*524289,{},400)]:
                req=urllib.request.Request(url+'/api/quest/frame',payload,headers|changed)
                with self.assertRaises(urllib.error.HTTPError) as error:urllib.request.urlopen(req)
                self.assertEqual(error.exception.code,code)
            frame=b'\xff\xd8test-frame\xff\xd9'
            with urllib.request.urlopen(urllib.request.Request(url+'/api/quest/frame',frame,headers)) as response:
                self.assertEqual(response.status,200)
            with urllib.request.urlopen(url+'/api/quest/frame?v=1') as response:
                self.assertEqual(response.read(),frame)
            self.assertTrue(state.snapshot()['quest']['streaming'])
            # Quest video never pretends that the editor control bridge is present.
            with self.assertRaises(ValueError):state.action('return')
            state.quest_frame_at-=5
            self.assertFalse(state.snapshot()['quest']['streaming'])
            with self.assertRaises(urllib.error.HTTPError) as error:urllib.request.urlopen(url+'/api/quest/frame')
            self.assertEqual(error.exception.code,503)
        finally:http.shutdown();http.server_close();thread.join()

    def test_quest_control_ack_expiry_and_source_isolation(self):
        s=State()
        s.quest_update(dict(send_enabled=False,can_return=True))
        s.action('send_on'); command=s.pending.copy()
        self.assertEqual(s.quest_update({})['action']['id'],command['id'])
        # An editor cannot consume or acknowledge a command intended for Quest.
        self.assertIsNone(s.bridge({'play':True,'ack':{'id':command['id']}})['action'])
        self.assertIsNotNone(s.pending)
        s.quest_update({'send_enabled':True,'ack':{'id':command['id'],'message':'enabled'}})
        self.assertIsNone(s.pending)
        self.assertTrue(s.snapshot()['quest']['control']['send_enabled'])
        s.quest_update({'can_return':True});s.action('return');s.action('send_off')
        self.assertEqual(s.pending['action'],'send_off')
        s.pending['created']-=3
        self.assertIsNone(s.quest_update({})['action'])
        s.quest_control_at-=4
        with self.assertRaises(ValueError):s.action('send_on')

    def test_receiver_start_guard_and_failure(self):
        s=State();calls=[]
        with self.assertRaises(ValueError):s.action('receiver_start')
        s.start_receiver=lambda:calls.append('start')
        s.quest_update({'send_enabled':True})
        with self.assertRaises(ValueError):s.action('receiver_start')
        self.assertEqual(calls,[])
        s.quest_update({'send_enabled':False});s.action('receiver_start')
        self.assertEqual(calls,['start'])
        with self.assertRaises(ValueError):s.action('return')

    def test_quest_control_http_permissions(self):
        s=State();http=ThreadingHTTPServer(('127.0.0.1',0),handler(s,'secret',0))
        port=http.server_port;http.RequestHandlerClass=handler(s,'secret',port)
        thread=threading.Thread(target=http.serve_forever,daemon=True);thread.start()
        try:
            url=f'http://127.0.0.1:{port}/api/quest/control'
            for headers in [{},{'X-Quest-Stream':'1','Origin':'https://evil.example'}]:
                with self.assertRaises(urllib.error.HTTPError) as err:
                    urllib.request.urlopen(urllib.request.Request(url,b'{}',headers))
                self.assertEqual(err.exception.code,403)
            with urllib.request.urlopen(urllib.request.Request(url,b'{"can_return":true}',{'X-Quest-Stream':'1'})) as res:
                self.assertIsNone(json.load(res)['action'])
            s.action('return')
            with urllib.request.urlopen(urllib.request.Request(url,b'{}',{'X-Quest-Stream':'1'})) as res:
                self.assertEqual(json.load(res)['action']['action'],'return')
        finally:http.shutdown();http.server_close();thread.join()

    def test_stale_receiver_is_hidden(self):
        s=State();s.connected=True
        data=dict(l=.2,r=.2,th=1,st=0,dry_run=False,live=True)
        s.telemetry(data)
        self.assertIsNotNone(s.snapshot()['receiver'])
        s.received-=2
        self.assertIsNone(s.snapshot()['receiver'])
        s.telemetry(data,retained=True)
        self.assertIsNone(s.snapshot()['receiver'])

    def test_invalid_telemetry(self):
        s=State()
        for value in [float('nan'),True,2,'0.2']:
            s.telemetry(dict(l=value,r=0,th=0,st=0,dry_run=False,live=True))
            self.assertIsNone(s.receiver)

    def test_commands_need_live_play_and_ack(self):
        s=State()
        with self.assertRaises(ValueError):s.action('return')
        s.bridge({'play':True})
        s.action('return');command=s.pending.copy()
        s.bridge({'play':True,'ack':{'id':'wrong'}})
        self.assertIsNotNone(s.pending)
        s.bridge({'play':True,'ack':{'id':command['id'],'message':'accepted'}})
        self.assertIsNone(s.pending)
        s.action('return');s.pending['created']-=3
        self.assertFalse(s.snapshot()['pending'])

    def test_stop_supersedes_pending_return(self):
        s=State();s.bridge({'play':True});s.action('return');s.action('stop')
        self.assertEqual(s.pending['action'],'stop')

    def test_http_rejects_cross_origin_and_bad_token(self):
        s=State();http=ThreadingHTTPServer(('127.0.0.1',0),handler(s,'secret',0))
        port=http.server_port;http.RequestHandlerClass=handler(s,'secret',port)
        thread=threading.Thread(target=http.serve_forever,daemon=True);thread.start()
        try:
            for path,headers in [('/api/action',{'X-Dashboard':'local','Origin':'https://example.com'}),('/api/bridge',{'X-Bridge-Token':'wrong'})]:
                request=urllib.request.Request(f'http://127.0.0.1:{port}'+path,b'{}',headers)
                with self.assertRaises(urllib.error.HTTPError) as error:urllib.request.urlopen(request)
                self.assertEqual(error.exception.code,403)
            with urllib.request.urlopen(f'http://127.0.0.1:{port}/api/state') as response:
                self.assertFalse(json.load(response)['broker'])
        finally:http.shutdown();http.server_close();thread.join()


    def test_lan_host_origin_and_bridge_authentication(self):
        state=State();http=ThreadingHTTPServer(('127.0.0.1',0),handler(state,'secret',0))
        port=http.server_port
        http.RequestHandlerClass=handler(state,'secret',port,'10.95.86.138')
        thread=threading.Thread(target=http.serve_forever,daemon=True);thread.start()
        try:
            url=f'http://127.0.0.1:{port}'
            headers={'Host':f'10.95.86.138:{port}', 'Origin':f'http://10.95.86.138:{port}', 'X-Bridge-Token':'secret'}
            request=urllib.request.Request(url+'/api/bridge',b'{"play":false}',headers)
            with urllib.request.urlopen(request) as response:
                self.assertEqual(response.status,200)
                self.assertIn('ws://10.95.86.138:8889',response.headers['Content-Security-Policy'])
            for changes in [{'Host':f'evil.example:{port}'},{'Origin':'https://evil.example'},{'X-Bridge-Token':'wrong'}]:
                request=urllib.request.Request(url+'/api/bridge',b'{}',headers|changes)
                with self.assertRaises(urllib.error.HTTPError) as error:urllib.request.urlopen(request)
                self.assertEqual(error.exception.code,403)
        finally:http.shutdown();http.server_close();thread.join()



class VideoTests(unittest.TestCase):
    @staticmethod
    def packet(i, key=False):
        import struct
        data=(b'\0\0\0\1\x67\x42\xc0\x1f\x01\0\0\0\1\x68\x01' if key else b'')
        data+=b'\0\0\0\1'+(b'\x65' if key else b'\x41')+b'\x01'
        return struct.pack('<QI',i*33333,len(data))+data

    def test_video_keyframe_reconnect_bounds_and_control_isolation(self):
        s=State();v=s.video
        for i in range(180):v.ingest('a'*32,960,540,self.packet(i,i%30==0))
        self.assertLessEqual(len(v.frames),90)
        self.assertLessEqual(v.bytes,4194304)
        reply=v.read('',0)
        self.assertTrue(reply['reset']);self.assertTrue(reply['init'])
        self.assertEqual(reply['frames'][0]['id'],151)
        self.assertFalse(v.read('a'*32,reply['frames'][-1]['id'])['reset'])
        self.assertIsNone(s.pending);self.assertIsNone(s.receiver)
        v.at-=4;self.assertFalse(v.status()['live'])

    def test_video_rejects_truncation_and_waits_for_keyframe(self):
        from quest_video import QuestVideo
        v=QuestVideo()
        for body in [b'',self.packet(0,True)[:-1],b'garbage']:
            with self.assertRaises(ValueError):v.ingest('a'*32,960,540,body)
        self.assertFalse(v.init)
        v.ingest('a'*32,960,540,self.packet(0))
        self.assertFalse(v.init)
        v.ingest('a'*32,960,540,self.packet(1,True))
        v.ingest('a'*32,960,540,self.packet(2))
        self.assertTrue(v.status()['live'])
        with self.assertRaises(ValueError):v.ingest('a'*32,960,540,self.packet(2))
        v.ingest('a'*32,960,540,self.packet(100))
        self.assertFalse(v.status()['live'])


class CaptionTests(unittest.TestCase):
    def test_caption_expiry_clear_and_stale_link(self):
        with patch('server.time.monotonic',return_value=100) as clock:
            s=State();s.quest_update({'notices':[{'kind':'return','text':'자동복귀를 시작합니다','remaining':2}], 'notice_badge':'자동복귀 중'})
            clock.return_value=101
            q=s.snapshot()['quest'];self.assertEqual(q['notices'][0]['remaining'],1)
            clock.return_value=102.1
            self.assertEqual(s.snapshot()['quest']['notices'],[])
            self.assertEqual(s.snapshot()['quest']['notice_badge'],'자동복귀 중')
            clock.return_value=103.1
            self.assertEqual(s.snapshot()['quest']['notice_badge'],'')
            self.assertEqual(s.snapshot()['quest']['notice_lease'],0)
            s.quest_update({});self.assertEqual(s.quest_notices,[])
            self.assertIsNone(s.pending)

    def test_caption_validation_and_same_text_reentry(self):
        with patch('server.time.monotonic',return_value=100) as clock:
            s=State()
            for ttl in [float('nan'),float('inf'),True,-1,'3']:
                s.quest_update({'notices':[{'kind':'return','text':'test','remaining':ttl}]})
                self.assertEqual(s.quest_notices,[])
            s.quest_update({'notices':[None,{'kind':'unknown','text':'x','remaining':3},{'kind':'whale','text':'x'*600,'remaining':100}], 'notice_badge':{}})
            self.assertEqual(len(s.quest_notices),1);self.assertEqual(len(s.quest_notices[0]['text']),500)
            self.assertEqual(s.quest_notices[0]['until'],105);self.assertEqual(s.quest_badge,'')
            clock.return_value=110
            s.quest_update({'notices':[{'kind':'whale','text':'same','remaining':2}]})
            self.assertEqual(s.snapshot()['quest']['notices'][0]['text'],'same')
            s.quest_update({'notices':[]});self.assertEqual(s.snapshot()['quest']['notices'],[])
            clock.return_value=111
            s.quest_update({'notices':[{'kind':'whale','text':'same','remaining':2}]})
            self.assertEqual(s.snapshot()['quest']['notices'][0]['remaining'],2)

if __name__=='__main__':unittest.main()
