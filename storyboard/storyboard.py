#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import xml.sax

class Vistor(xml.sax.ContentHandler):
    def __init__(self):
        self.tags = {}
        self.actions = []
        self.propertys = []
        self.outlets = []
        self.customClass = ''
    def startElement(self, tag, attributes):
        if tag == 'viewController':
            self.customClass = attributes['customClass']
            self.tags[attributes['id']] = self.customClass
        elif tag == 'action' and attributes.has_key('selector'):
            self.actions.append((attributes['selector'], attributes['destination']))
        elif tag == 'outlet' and attributes.has_key('property'):
            self.propertys.append(attributes['property'])
    def endElement(self, tag):
        if tag == 'viewController':
            self.outlets.append((self.customClass, self.propertys))
            self.propertys = []
            self.customClass = ''
    def endDocument(self):
        for action in self.actions:
            print '-[{0} {1}]'.format(self.tags[action[1]], action[0])
        for outlet in self.outlets:
            for prop in outlet[1]:
                print '-[{0} get{1}:]'.format(outlet[0], prop.capitalize())
                print '-[{0} set{1}:]'.format(outlet[0], prop.capitalize())

def handleStoryboard(path):
    parser = xml.sax.make_parser()
    parser.setFeature(xml.sax.handler.feature_namespaces, 0)
    visitor = Vistor()
    parser.setContentHandler(visitor)
    parser.parse(path)

for i in range(1, len(sys.argv)):
    if os.path.exists(sys.argv[i]):
        handleStoryboard(sys.argv[i])