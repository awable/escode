# coding: utf8
#!/usr/bin/env python
from unittest import TestCase

import escode


class TestArray(TestCase):
    def setUp(self):
        hemingway = u"""
        The hills across the valley of the Ebro' were long and white. On this side
        there was no shade and no trees and the station was between two lines of
        rails in the sun. Close against the side of the station there was the warm
        shadow of the building and a curtain, made of strings of bamboo beads,
        hung across the open door into the bar, to keep out flies. The American and
        the girl with him sat at a table in the shade, outside the building. It was very
        hot and the express from Barcelona would come in forty minutes. It
        stopped at this junction for two minutes and went on to Madrid.
        "What should we drink?" the girl asked. She had taken off her hat and
        put it on the table.
        "It's pretty hot," the man said.
        "Let's drink beer."
        "Dos cervezas," the man said into the curtain.
        "Big ones?" a woman asked from the doorway.
        "Yes. Two big ones."
        The woman brought two glasses of beer and two felt pads. She put the
        felt pads and the beer glasses on the table and looked at the man and the
        girl. The girl was looking off at the line of hills. They were white in the sun
        and the country was brown and dry.
        "They look like white elephants," she said.
        "I've never seen one," the man drank his beer.
        "No, you wouldn't have."
        "I might have," the man said. "Just because you say I wouldn't have
        doesn't prove anything."
        The girl looked at the bead curtain. "They've painted something on it,"
        she said. "What does it say?"
        "Anis del Toro. It's a drink."
        "Could we try it?"
        The man called "Listen" through the curtain. The woman came out
        from the bar.
        "Four reales."
        "We want two Anis del Toro."
        "With water?"
        "Do you want it with water?"
        "I don't know," the girl said. "Is it good with water?"
        "It's all right."
        "You want them with water?" asked the woman.
        "Yes, with water."
        "It tastes like licorice," the girl said and put the glass down.
        "That's the way with everything."
        "Yes," said the girl. "Everything tastes of licorice. Especially all the
        things you've waited so long for, like absinthe."
        "Oh, cut it out."
        "You started it," the girl said. "I was being amused. I was having a fine
        time."
        "Well, let's try and have a fine time."
        "All right. I was trying. I said the mountains looked like white elephants. Wasn't that bright?"
        "That was bright."
        "I wanted to try this new drink. That's all we do, isn't it—look at things
        and try new drinks?"
        " I guess so."
        The girl looked across at the hills.
        "They're lovely hills," she said. "They don't really look like white elephants. I just meant the coloring of their skin through the trees." "Should
        we have another drink?"
        "All right."
        The warm wind blew the bead curtain against the table.
        "The beer's nice and cool," the man said.
        "It's lovely," the girl said.
        "It's really an awfully simple operation, Jig," the man said. "It's not
        really an operation at all."
        The girl looked at the ground the table legs rested on.
        " I know you wouldn't mind it, Jig. It's really not anything. It's just to let
        the air in."
        The girl did not say anything.
        "I'll go with you and I'll stay with you all the time. They just let the air in
        and then it's all perfectly natural."
        "Then what will we do afterward?"
        "We'll be fine afterward. Just like we were before."
        "What makes you think so?"
        "That's the only thing that bothers us. It's the only thing that's made us
        unhappy."
        The girl looked at the bead curtain, put her hand out and took hold of
        two of the strings of beads.
        "And you think then we'll be all right and be happy."
        "I know we will. You don't have to be afraid. I've known lots of people
        that have done it."
        "So have I," said the girl. "And afterward they were all so happy."
        "Well," the man said, "if you don't want to you don't have to. I wouldn't
        have you do it if you didn't want to. But I know it's perfectly simple."
        "And you really want to?"
        """.strip().split(u"\n")
        self.doc = {u"hemingway": hemingway}

    def test_long_array(self):
        serialized = escode.encode(self.doc)
        doc2 = escode.decode(serialized)
        self.assertEquals(self.doc, doc2)
