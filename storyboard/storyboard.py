#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import xml.sax

class Vistor(xml.sax.ContentHandler):
    def __init__(self):
        self.tags = {}
        self.actions = []
    def startElement(self, tag, attributes):
        if tag == 'viewController':
            self.tags[attributes['id']] = attributes['customClass']
        elif tag == 'action' and attributes.has_key('selector'):
            self.actions.append((attributes['selector'], attributes['destination']))
    def endDocument(self):
        for action in self.actions:
            print '-[{0} {1}]'.format(self.tags[action[1]], action[0])

def handleStoryboard(path):
    parser = xml.sax.make_parser()
    parser.setFeature(xml.sax.handler.feature_namespaces, 0)
    visitor = Vistor()
    parser.setContentHandler(visitor)
    parser.parse(path)

for i in range(1, len(sys.argv)):
    if os.path.exists(sys.argv[i]):
        handleStoryboard(sys.argv[i])