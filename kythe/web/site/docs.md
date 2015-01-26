---
layout: page
title: Documentation
permalink: /docs/
order: 10
---

{% comment %}
TODO(schroederc): categorize documentation
{% endcomment %}

{% assign docs = site.docs | sort:"priority" | reverse %}
{% for doc in docs %}
- [{{ doc.title }}]({{ doc.url }})
{% endfor %}
