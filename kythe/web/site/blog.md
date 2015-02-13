---
layout: page
title: Blog
permalink: /blog/
order: 0
---

{% for post in site.posts %}
- <span class="post-meta">{{ post.date | date: '%Y-%m-%d' }}</span> » [{{ post.title }}]({{ post.url }}) -- {{ post.excerpt | strip_html }}
{% endfor %}
